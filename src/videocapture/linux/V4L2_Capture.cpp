#include <videocapture/linux/V4L2_Capture.h>

namespace ca {

  // WRAPPER - see: https://gist.github.com/maxlapshin/1253534
  int v4l2_ioctl(int fh, int request, void* arg) {
    int r;

    do {
      r = ioctl(fh, request, arg);
    } while(-1 == r && EINTR == errno);

    return r;
  }


  /* INTERFACE IMPLEMENTATION */
  /* -------------------------------------- */
  V4L2_Capture::V4L2_Capture(frame_callback fc, void* user)
    :Base(fc, user)
    ,state(CA_STATE_NONE)
    ,capture_device_fd(-1)
  {
    pixel_buffer.user = user;
  }

  V4L2_Capture::~V4L2_Capture() {

    if(state & CA_STATE_CAPTUREING) {
      stop();
    }

    if(state & CA_STATE_OPENED) {
      close();
    }

    state = CA_STATE_NONE;
    capture_device_fd = -1;
    pixel_buffer.user = NULL;
  }

  int V4L2_Capture::open(Settings settings) {

    // Already open?
    if(capture_device_fd > 0) {
      printf("Error: device already opened. First close it.\n");
      return -1;
    }

    if(state & CA_STATE_OPENED) {
      printf("Error: already openend.\n");
      return -2;
    }

    // Get all devices (we select the one set in `settings`).
    std::vector<V4L2_Device> v4l2_devices = v4l2_get_devices(); 
    if(settings.device >= v4l2_devices.size()) {
      printf("Error: device index is invalid.\n");
      return -3;
    }

    // Get capabilities (we need these to set the specs).
    std::vector<Capability> capabilities = getCapabilities(settings.device);

    if(settings.capability >= capabilities.size()) {
      printf("Error: Invalid capability index.\n");
      return -4;
    }

    Capability& cap = capabilities.at(settings.capability);

    // Get device
    V4L2_Device v4l2_device = v4l2_devices.at(settings.device);
    capture_device_fd = openDevice(v4l2_device.path);

    if(capture_device_fd < 0) {
      printf("Error: cannot open the device: %d\n", capture_device_fd);
      closeDevice(capture_device_fd);
      return -5;
    }

    // Set the capture format.
    int pix_fmt = capture_format_to_v4l2_pixel_format(cap.pixel_format);

    if(pix_fmt == CA_NONE) {
      std::string str = format_to_string(cap.pixel_format).c_str();
      printf("Error: cannot find the v4l2 pixel format for the capture format: %s\n", str.c_str());
      closeDevice(capture_device_fd);
      return -6;
    }

    if(setCaptureFormat(capture_device_fd, cap.width, cap.height, pix_fmt) < 0) {
      printf("Error: cannot set the capture format.\n");
      closeDevice(capture_device_fd);
      return -7;
    }

    // Get capabilities (test if we can stream).
    struct v4l2_capability caps;

    if(!getCapabilityV4L2(capture_device_fd, &caps)) {
      closeDevice(capture_device_fd);
      return -8;
    }

    if(!(caps.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
      printf("Error: Not a video capture device; we only support video capture devices.\n");
      closeDevice(capture_device_fd);
      return -9;
    }

    // check io
    bool can_io_readwrite = (caps.capabilities & V4L2_CAP_READWRITE) == V4L2_CAP_READWRITE;
    bool can_io_stream = (caps.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING;

    if(!can_io_readwrite && !can_io_stream) {
      printf("Error: Cannot use read() or memory streaming with this device.\n");
      closeDevice(capture_device_fd);
      return -10;
    }

    if(!can_io_stream) {
      printf("Error: The device cannot memory stream; we only support this method for now.\n");
      closeDevice(capture_device_fd);
      return -11;
    }
    
    if(initializeMMAP(capture_device_fd) < 0) {
      return -12;
    }

    state |= CA_STATE_OPENED;

    pixel_buffer.pixel_format = cap.pixel_format;

    return 1;
  }

  int V4L2_Capture::close() {

    if(capture_device_fd < 0) {
      printf("Error: cannot close capture because it's not opened. Invalid fd.\n");
      return -1;
    }

    if( (state & CA_STATE_OPENED) != CA_STATE_OPENED) {
      printf("Error: cannot close a capture device because it's not opened.\n");
      return -2;
    }

    if(closeDevice(capture_device_fd) < 0) {
      if(shutdownMMAP()) {
        return -3;
      }
      return -4;
    }

    if(shutdownMMAP() < 0) {
      return -5;
    }

    state &= ~CA_STATE_OPENED;
    return 1;
  }

  // Start streaming.
  int V4L2_Capture::start() {

    if(capture_device_fd < 0) {
      printf("Error: cannot start capture as the device descriptor is invalid.\n");
      return -1;
    }

    if(state & CA_STATE_CAPTUREING) {
      printf("Error: we are already captureing. Cannot start again.\n");
      return -2;
    }

    if((state & CA_STATE_OPENED) != CA_STATE_OPENED) {
      printf("Error: not yet opened.\n");
      return -3;
    }
  
    for(int i = 0; i < (int)buffers.size(); ++i) {
      struct v4l2_buffer buf;
      memset(&buf, 0, sizeof(buf));
    
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = i;
    
      if(v4l2_ioctl(capture_device_fd, VIDIOC_QBUF, &buf) == -1) {
        printf("Error: VIDIO_QBUF failed - invalid mmap buffer.\n");
        return -4;
      }
    }

    // stream on!
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(v4l2_ioctl(capture_device_fd, VIDIOC_STREAMON, &type) == -1) {
      printf("Error: Failed to start the video capture stream, VIDIOC_STREAMON failed.\n");
      return -5;
    }

    state |= CA_STATE_CAPTUREING;

    return 1;
  }

  int V4L2_Capture::stop() {

    if(capture_device_fd < 0) {
      printf("Error: cannot stop captureing becuause the device descriptor is invalid.\n");
      return -1;
    }

    if( (state & CA_STATE_CAPTUREING) != CA_STATE_CAPTUREING) {
      printf("Error: cannot stop captureing because we didn't start captureing yet.\n");
      return -2;
    }

    // stream off!
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(v4l2_ioctl(capture_device_fd, VIDIOC_STREAMOFF, &type) == -1) {
      printf("Error: cannot stop captureing because of an ioctl error. (did you really start capturing before?).\n");
      return -3;
    }

    state &= ~CA_STATE_CAPTUREING;

    return 1;
  }

  void V4L2_Capture::update() {

    readFrame();
  }

  // Read one more frame.
  int V4L2_Capture::readFrame() {
    
    if(capture_device_fd < 0) {
      printf("Error: cannot readFrame() because the capture device descriptor is invalid.\n");
      return -1;
    }

    /* @todo do we really need to memset the buff to zero every frame? */
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
  
    if(v4l2_ioctl(capture_device_fd, VIDIOC_DQBUF, &buf) == -1) {
      if(errno == EAGAIN) {
        return -2; /* everything ok; just not ready yet */
      }
      else if(errno == EIO) {
        printf("Error: IO error.\n");
        return -3; /* we could handle this as an error. */
      }
      else {
        printf("Error: with reading the memory buffer: %s.\n", strerror(errno));
        return -4;
      }
    }

    assert(buf.index < buffers.size());

    if(cb_frame) {
      pixel_buffer.pixels = (uint8_t*)buffers[buf.index]->start;
      pixel_buffer.plane[0] = pixel_buffer.pixels;
      pixel_buffer.nbytes = buf.bytesused;
      cb_frame(pixel_buffer);
    }

    if(v4l2_ioctl(capture_device_fd, VIDIOC_QBUF, &buf) == -1) {
      printf("Error: with queueing the buffer again: %s.\n", strerror(errno));
      return -5;
    }

    return 1;
  }

  std::vector<Capability> V4L2_Capture::getCapabilities(int device) {

    std::vector<Capability> result;
    struct v4l2_capability cap;
    struct v4l2_standard standard;
    struct v4l2_input input;
    struct v4l2_fmtdesc fmtdesc;
    struct v4l2_format format;

    memset(&cap, 0, sizeof(cap));

    V4L2_Device v4l2_device;

    if(getDeviceV4L2(device, v4l2_device) < 0) {
      printf("Error: Cannot find the input device to list capabilities.\n");
      return result;
    }

    int fd = openDevice(v4l2_device.path);

    if(fd < 0) {
      return result;
    }

    if(v4l2_ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
      printf("Error: Cannot query the device capabilities.\n");
      return result;
    }

    // capabilities
    if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {

      for(int i = 0; ; ++i) {

        memset(&fmtdesc, 0, sizeof(fmtdesc));
        fmtdesc.index = i;
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if(v4l2_ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == -1) {
          return result;
        }

        Capability capability;

        // frame sizes and fps for this pixel format
        struct v4l2_frmsizeenum frames;
        memset(&frames, 0x00, sizeof(frames));
        frames.index = 0;
        frames.pixel_format = fmtdesc.pixelformat;

        while(!v4l2_ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frames)) {

          if(frames.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
            struct v4l2_frmivalenum fpse;
            memset(&fpse, 0x00, sizeof(fpse));
            fpse.index = 0;
            fpse.pixel_format = frames.pixel_format;
            fpse.width = frames.discrete.width;
            fpse.height = frames.discrete.height;

            while(!v4l2_ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &fpse)) {
              if(fpse.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
                capability.fps = fps_from_rational((uint64_t)fpse.discrete.numerator, (uint64_t)fpse.discrete.denominator);
                capability.width = frames.discrete.width;
                capability.height = frames.discrete.height;
                capability.pixel_format = v4l2_pixel_format_to_capture_format(frames.pixel_format);
                capability.capability_index = result.size();
                capability.fps_index = fpse.index;
                capability.pixel_format_index = i;
                result.push_back(capability);
              }
              fpse.index++;
            }
          }
          frames.index++;
        }
      } // for
    }

    closeDevice(fd);

    return result;
  }

  std::vector<Device> V4L2_Capture::getDevices() {

    std::vector<Device> result;
    std::vector<V4L2_Device> devs = v4l2_get_devices();

    for(size_t i = 0; i < devs.size(); ++i) {
      
      V4L2_Device& v4l2_dev = devs[i];
      Device dev;

      if (getDriverInfo(v4l2_dev.path.c_str(), v4l2_dev) < 0) {
        printf("We didn't find any driver info for the device: %s.\n", v4l2_dev.path.c_str());
        continue;
      }
      
      dev.index = i;
      dev.name = v4l2_dev.toString();
      
      result.push_back(dev);
    }

    return result;
  }

  std::vector<Format> V4L2_Capture::getOutputFormats() {
    std::vector<Format> result;
    return result;
  }

  /* V4L2 IMPLEMENTATION */
  /* -------------------------------------- */

  // Allocate the MMAP'd buffers.
  int V4L2_Capture::initializeMMAP(int fd) {

    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if(v4l2_ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
      printf("Error: Cannot use mmap().\n");
      return -1;
    }

    if(req.count < 2) {
      printf("Error: Insufficient buffer memory.\n");
      return -2;
    }

    for(int i = 0; i < req.count; ++i) {

      // Create our reference to the mmap'd buffer.
      V4L2_Buffer* buffer = new V4L2_Buffer();

      if(!buffer) {
        printf("Error: Cannot allocate the V4L2 buffer for mmap'd IO.\n");
        goto error;
      }

      buffers.push_back(buffer);

      // Create the v4l2 buffer.
      struct v4l2_buffer vbuf;
      memset(&vbuf, 0, sizeof(vbuf));
      vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      vbuf.memory = V4L2_MEMORY_MMAP;
      vbuf.index = i;

      // map the buffer.
      if(v4l2_ioctl(fd, VIDIOC_QUERYBUF, &vbuf) == -1) {
        printf("Error: Cannot query the buffer for index: %d.\n", vbuf.index);
        goto error;
      }

      buffer->length = vbuf.length;
      buffer->start = mmap(NULL, /* start anywhere */
                           vbuf.length,
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED,
                           fd, vbuf.m.offset);

      if(buffer->start == MAP_FAILED) {
        if(errno == EBADF) {
          printf("Error: cannot map memory, fd is not a valid descriptor. (EBADF).\n");
        }
        else if(errno == EACCES) {
          printf("Error: cannot map memory, fd is open for reading and writing. (EACCESS).\n");
        }
        else if(errno == EINVAL) {
          printf("Error: cannot map memory, the start or length offset are not suitable. Flags or prot value is not supported. No buffers have been allocated. (EINVAL).\n");
        }
        else {
          printf("Error: MMAP failed.\n");
        }
        goto error;
      }
    
    } // for 

    return 1;

  error:
    shutdownMMAP();
    return -1;
  }
   
  // Free MMAP'd memory
  int V4L2_Capture::shutdownMMAP() {

    for(std::vector<V4L2_Buffer*>::iterator it = buffers.begin(); it != buffers.end(); ++it) {
      V4L2_Buffer* buf = *it;
      if(munmap(buf->start, buf->length) == -1) {
        printf("Error: cannot unmap a memory buffer (?)\n");
      }
      delete buf;
    }

    buffers.clear();
    return true;
  }
  
  // Set the given width/height/pixfm for the fd (capture device).
  int V4L2_Capture::setCaptureFormat(int fd, int width, int height, int pixfmt) { 

    if(fd <= 0) {
      printf("Error: cannot set capture format, invalid fd.\n");
      return -1;
    }

    // As described in section 4.1.3, http://www.linuxtv.org/downloads/legacy/video4linux/API/V4L2_API/spec-single/v4l2.html#id2832544
    // we first retrieve the current pixel format and then adjust the settings to our needs before
    // trying the format.
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if(v4l2_ioctl(fd, VIDIOC_G_FMT, &fmt) == -1) {
      printf("Error: cannot retrieve the current format that we need to change/set it.\n");
      return -2;
    }

    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = pixfmt;

    if(fmt.fmt.pix.pixelformat == 0) {
      printf("Error: cannot find a v4l2 pixel format for the given a LibAV PixelFormat.\n");
      return -3;
    }
   
    // try the new format
    if(v4l2_ioctl(fd, VIDIOC_TRY_FMT, &fmt) == -1) {
      printf("Error: the video capture device doesnt support the given format.\n");
      return -4;
    }

    // set the new format
    if(v4l2_ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
      printf("Error: cannot set the given format %d x %d, (%s).\n", width, height, strerror(errno));
      return -5;
    }

    return 1;
  }

  int V4L2_Capture::getDeviceV4L2(int dx, V4L2_Device& result) {

    std::vector<V4L2_Device> devices = v4l2_get_devices();

    if(dx > (int)devices.size()-1) {
      printf("Error: Device not found for index %d. Are you sure you're using a valid index?\n", dx);
      return -1;
    }

    result = devices[dx];

    return 1;
  }

  int V4L2_Capture::getCapabilityV4L2(int fd, struct v4l2_capability* caps) {

    if(fd == 0 || fd < 0) {
      printf("Error: cannot get capablity; invalid fd.\n");
      return -1;
    }

    memset(caps, 0, sizeof(*caps));
    if(v4l2_ioctl(fd, VIDIOC_QUERYCAP, caps) == -1) {
      printf("Cannot query capability for: %d.\n", fd);
      return -2;
    }

    return 1;
  }

  // Open the given device (by path) , returns the file descriptor or < 0 on error
  int V4L2_Capture::openDevice(std::string path) {

    if(!path.size()) {
      printf("Error: cannot open the V4L2 Device, Wrong path.\n");
      return -1;
    }

    int fd = ::open(path.c_str(), O_RDWR | O_NONBLOCK, 0);
    if(fd == -1) {
      printf("Error: Cannot open V4L2 Device: %s", path.c_str());
      return -2;
    }

    return fd;
  }

  int V4L2_Capture::closeDevice(int fd) {

    if(fd < 0) {
      printf("Error: wrong file descriptor; cannot close device.\n");
      return fd;
    }

    if(::close(fd) == -1) {
      printf("Error: something went wrong while trying to close the device.\n");
      return -1;
    }

    fd = -1;

    return 1;
  }

  int V4L2_Capture::getDriverInfo(const char* path, V4L2_Device& result) {
    
    int fd = openDevice(path);
    if(fd < 0) {
      return fd;
    }

    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(cap));

    if(v4l2_ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
      printf("Error: Cannot query the device capabilities.\n");
      closeDevice(fd);
      return -2;
    }

    result.driver.assign((char*)cap.driver, strlen((char*)cap.driver));
    result.card.assign((char*)cap.card, strlen((char*)cap.card));
    result.bus_info.assign((char*)cap.bus_info, strlen((char*)cap.bus_info));;
    result.version_major = (cap.version >> 16) & 0xFF;
    result.version_minor = (cap.version >> 8) & 0xFF;
    result.version_micro = (cap.version & 0xFF);

    closeDevice(fd);
    return 1;
  }
} // namespace ca
