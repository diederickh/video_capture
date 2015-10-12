/*

  V4L2_Capture
  ------------

  Video4Linux2 Capture wrapper. 
  
 */
#ifndef VIDEO_CAPTURE_V4L2_CAPTURE_H
#define VIDEO_CAPTURE_V4L2_CAPTURE_H

extern "C" {
#  include <assert.h>
#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#  include <fcntl.h>
#  include <errno.h>
#  include <sys/mman.h>
#  include <sys/ioctl.h>
#  include <asm/types.h>
#  include <linux/videodev2.h>
#  include <locale.h>
#  include <unistd.h>
}

#include <string>
#include <vector>
#include <videocapture/Base.h>
#include <videocapture/Types.h>
#include <videocapture/Utils.h>
#include <videocapture/linux/V4L2_Types.h>
#include <videocapture/linux/V4L2_Utils.h>
#include <videocapture/linux/V4L2_Devices.h>

namespace ca {

  int v4l2_ioctl(int fh, int request, void* arg);                                      /* Wrapper around ioctl */

  class V4L2_Capture : public Base {

  public:
    V4L2_Capture(frame_callback fc, void* user);
    ~V4L2_Capture();

    /* Interface */
    int open(Settings settings);                                                       /* Open the given device */
    int close();                                                                       /* Close the previously opened device */
    int start();                                                                       /* Start captureing */ 
    int stop();                                                                        /* Stop captureing. */
    void update();                                                                     /* This should be called at framerate; this will grab a new frame */

    /* Capabilities */
    std::vector<Capability> getCapabilities(int device);                               /* Get all the capabilities for the given device number  */
    std::vector<Device> getDevices();                                                  /* We query the udev USB devices. */
    std::vector<Format> getOutputFormats();                                            /* Get the supported output formats. For V4L2 this is empty. */

    /* IO Methods */
    int initializeMMAP(int fd);                                                        /* Initialize MMAP I/O for the given file descriptor */
    int shutdownMMAP();                                                                /* Shutdown MMAP and free all buffers. */
    int readFrame();                                                                   /* Reads one frame from the device */

    /* Device related*/
    int openDevice(std::string path);                                                  /* Open the device and return a descriptor; path is the device devpat.h */
    int closeDevice(int fd);                                                           /* Close the given device descriptor; is use when opening/closing multiple devices to test e.g. capabilities; get info on the devices, etc.. */
    int getDriverInfo(const char* path, V4L2_Device& result);                          /* Get extra driver info for the given syspath. */
    int setCaptureFormat(int fd, int width, int height, int pixfmt);                   /* Set the pixel format for the given fd, widht, height and pixfmt. */
    int getDeviceV4L2(int dx, V4L2_Device& result);                                    /* Get the device for the given index */
    int getCapabilityV4L2(int fd, struct v4l2_capability* caps);                       /* Get a v4l2_capability object for the given fd. */

  private:
    int state;                                                                         /* We keep track of the open/capture state so we know when to stop/close the device */
    int capture_device_fd;                                                             /* File descriptor for the capture device. */
    std::vector<V4L2_Buffer*> buffers;                                                 /* The buffer that are used to store the frames from the capture device . */                                      
    PixelBuffer pixel_buffer;                                                          /* The object we pass to the callback. */
  };
}; // namespace ca

#endif
