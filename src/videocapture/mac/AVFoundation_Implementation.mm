/* -*-C-*- */
#import <videocapture/mac/AVFoundation_Implementation.h>

static void print_cmformatdescription_info(CMFormatDescriptionRef desc);

@implementation AVFoundation_Implementation 

// Initializes the capturer
- (id) init {

  self = [super init];

  if(self) {
    cb_frame = nil;
    cb_user= nil;
    session = nil;
    input = nil;
    output = nil;
    pixel_format = 0;
    is_pixel_buffer_set = 0;
  }

  return self;
}

// Deallocate all retained objects
- (void) dealloc {

  [self closeDevice];

  cb_frame = nil;
  cb_user = nil;
  pixel_buffer.user = NULL;

  [super dealloc];
}

// Open a capture device
- (int) openDevice: (ca::Settings) settings {

  if(input != nil) {
    printf("Error: device already opened.\n");
    return -1;
  }

  /* 

     @todo Mac supports automatical conversion of pixel formats. For example,
     the Logitech C920 cam, supports CA_YUYV422 but Mac can convert it to CA_UYVY422
     using the output format object. At the moment we only support capabilities that
     are supported by the cam itself. 

   */
  if (settings.capability < 0) {
    printf("Error: invalid capability: %d\n", settings.capability);
    return -2;
  }
  
  /* Get the capture device */
  AVCaptureDevice* cap_device = [self getCaptureDevice: settings.device];
  if(cap_device == nil) {
    printf("Error: cannot find the given capture device: %d\n", settings.device);
    return -3;
  }

  /* Get the input device */
  NSError* err = nil;
  input = [AVCaptureDeviceInput deviceInputWithDevice: cap_device error:&err];
  if(input == nil) {
    printf("Error: cannot get the capture input device: %d\n", settings.device);
    return -4;
  }

  [input retain];

  /* Create session */
  session = [[AVCaptureSession alloc] init];
  if(session == nil) {
    printf("Error: cannot create the capture session.\n");
    return -4;
  }

  /* Is set to the settings.capability values. */
  ca::Capability cap;

  [session beginConfiguration];
  {

    /* Get the capabilities for this device */
    std::vector<ca::Capability> capabilities;
    [self getCapabilities: capabilities forDevice: settings.device];
    assert(capabilities.size() > 0);
    cap = capabilities[settings.capability];

    /* Get the best matching session preset. */
    NSString* const preset = [self widthHeightToCaptureSessionPreset: cap.width andHeight: cap.height];
    if([session canSetSessionPreset: preset]) {
      session.sessionPreset = preset;
    }
    else {
      printf("Error: cannot set the session preset.\n");
      return -5;
    }

    /* Add the input */
    if([session canAddInput: input] == NO) {
      printf("Error: cannot add the device input to the session manager.\n");
      return -6;
    }
    
    [session addInput:input];

    /* Set the AVCaptureDeviceFormat and minFrameDuration */
    AVCaptureDeviceFormat* cap_format = [[cap_device formats] objectAtIndex: cap.pixel_format_index];
    AVFrameRateRange* fps = [[cap_format videoSupportedFrameRateRanges] objectAtIndex: cap.fps_index];

    if(cap_format != nil) {
      NSError* err = nil;
      [cap_device lockForConfiguration: &err];
      if(err) {
        printf("Error: cannot lock the capture device for configuration.\n");
        return -7;
      }

      [cap_device setActiveFormat: cap_format];
      [cap_device setActiveVideoMinFrameDuration: [fps minFrameDuration]];
      [cap_device unlockForConfiguration];
    }
    else {
      printf("Error: cannot find the capture format.\n");
      return  -8;
    }

    /* Create the output handler (needed for getOutputFormatormats) */
    if(output == nil) {
      output = [[AVCaptureVideoDataOutput alloc] init];
      if(!output) {
        printf("Error: cannot allocate the video data output\n");
        return -9;
      }
      [output retain];
    }

    /* Configure output */
    CMPixelFormatType fmt_type;
    std::vector<ca::Format> formats;
    [self getOutputFormats: formats];

    if(CA_NONE != settings.format) { 
      assert(formats.size() > 0);
      fmt_type = [self captureFormatToCvPixelFormat: settings.format]; 
    }
    else {
      fmt_type = [self captureFormatToCvPixelFormat: cap.pixel_format];
    }

    if (fmt_type == CA_NONE) {
      printf("Error: failed to convert a Capture pixel format type (e.g. CA_YUYV422, CA_*) to a cvPixel format that we need to configure the Mac/iOS capturer.\n");
      return -10;
    }

    /* Tell the AVCaptureVideoDataOutput to convert the incoming data if necessary (and specify w/h) */
    [output setVideoSettings: [NSDictionary dictionaryWithObjectsAndKeys: 
                               [NSNumber numberWithInt:fmt_type], kCVPixelBufferPixelFormatTypeKey,
                               [NSNumber numberWithInteger:cap.width], (id)kCVPixelBufferWidthKey,
                               [NSNumber numberWithInteger:cap.height], (id)kCVPixelBufferHeightKey,
                               nil]];

    /* 
       Cache the currently used pixel format that is used to fill the PixelBuffer 
       that we pass to the callback. We retrieve the pixel format from the current
       device to make sure that we're getting the one which is actually used. 
    */
    
    pixel_format = [self getCapturePixelFormat: fmt_type];

    if (pixel_format == CA_NONE) {
      printf("Error: failed to find the capture pixel format for the currently used cvPixel format.\n");
      return -11;
    }

    /* Setup framegrabber callback */
    dispatch_queue_t queue = dispatch_queue_create("Video Queue", DISPATCH_QUEUE_SERIAL);
    [output setSampleBufferDelegate:self queue:queue];

    if ([session canAddOutput:output] == NO) {
      printf("Error: cannot add the given output.\n");
      dispatch_release(queue);
      [session commitConfiguration];
      return -12;
    }

    dispatch_release(queue);
    [session addOutput:output]; 

  }

  [session commitConfiguration];

  return 1;
}

/* Close the device and shutdown the session. */
- (int) closeDevice {

  if(output != nil) {
    [output setSampleBufferDelegate:nil queue:dispatch_get_main_queue()];
  }
  
  if(session != nil) {

    if([session isRunning] == YES) {
      [session stopRunning];
    }

    if(input != nil) {
      [session removeInput: input];
    }

    if(output != nil) {
      [session removeOutput: output];
    }

    [session release];
  }

  
  if(input != nil) {
    [input release];
  }

  if(output != nil) {
    [output release];
  }

  input = nil;
  output = nil;
  session = nil;
  pixel_format = 0;
  is_pixel_buffer_set = 0;

  return 1;
}

/* Start the capture process. */
- (int) startCapture {

  if(session == nil) {
    printf("Error: cannot start the capture process, the session is not created.\n");
    return -1;
  }

  if(input == nil) {
    printf("Error: cannot start the capture process, the input device is nil.\n");
    return -2;
  }

  [session startRunning];

  return 1;
}

/* Stop the capture process. */
- (int) stopCapture {

  if(session == nil) {
    printf("Error: cannot stop the capture process, session is nil.\n");
    return -1;
  }

  if(input == nil) {
    printf("Error: cannot stop the capture process, input device is nil.\n");
    return -2;
  }

  if([session isRunning] == NO) {
    printf("Error: cannot stop the capture process, session is not running. First star() the session.\n");
    return -3;
  }

  [session stopRunning];
  return 1;
}

/* Set the frame callback. */
- (void) setCallback: (ca::frame_callback) cb user: (void*) u {
  cb_frame = cb;
  cb_user = u;
  pixel_buffer.user = u;
}

/* Capture callback. */
- (void) captureOutput: (AVCaptureOutput*) captureOutput
 didOutputSampleBuffer: (CMSampleBufferRef) sampleBuffer
        fromConnection: (AVCaptureConnection*) connection 
{

  if (nil == cb_frame) {
    printf("Error: capturing frames but the `cb_frame` callback is not set, not supposed to happen. Stopping\n");
    exit(EXIT_FAILURE);
  }

  CMFormatDescriptionRef desc = CMSampleBufferGetFormatDescription(sampleBuffer);
  FourCharCode fcc = CMFormatDescriptionGetMediaSubType(desc);

#if 0  
  printf("\nPrinting the received sampleBuffer format description.\n----------------------------------------------------------\n");
  print_cmformatdescription_info(desc);

  /* TMP */
  printf("Printing the device active format description.\n----------------------------------------------------------\n");
  CMFormatDescriptionRef ref = input.device.activeFormat.formatDescription;
  print_cmformatdescription_info(ref);
  /* END TMP */
  printf("==================\n\n");
#endif

  /* ------------------------------------------------------------------------------------------ */
  /* Compressed Formats                                                                         */
  /* ------------------------------------------------------------------------------------------ */
  
  if (kCMVideoCodecType_JPEG_OpenDML == fcc) {

    CMBlockBufferRef block_buffer = CMSampleBufferGetDataBuffer(sampleBuffer);
    if (NULL == block_buffer) {
      printf("Error: failed to get access to the block buffer for JPEG data.\n");
      return;
    }

    size_t len_at_offset = 0;
    size_t total_length = 0;
    char* data_pointer = NULL;
    bool is_contiguous = false;
    OSStatus status;
    
    status = CMBlockBufferGetDataPointer(block_buffer, 0, &len_at_offset, &total_length, &data_pointer);
    
    if (kCMBlockBufferNoErr != status) {
      printf("Error: failed to get a pointer to the data pointer.\n");
      return;
    }

    if (len_at_offset != total_length) {
      printf("Error: the length at the offset of the data pointer is not the same as the total frame size. We're not handling this situation yet.\n");
      return;
    }

    is_contiguous = CMBlockBufferIsRangeContiguous(block_buffer,  0, total_length);
    if (false == is_contiguous) {
      printf("Error: the received datablock is not contiguous which we expect it to be.\n");
      return;
    }

    if (0 == is_pixel_buffer_set) {
      CMFormatDescriptionRef desc = CMSampleBufferGetFormatDescription(sampleBuffer);
      CMVideoDimensions dims = CMVideoFormatDescriptionGetDimensions(desc);
      CMPixelFormatType pix_fmt = CMFormatDescriptionGetMediaSubType(desc);
      
      pixel_buffer.width[0] = dims.width;
      pixel_buffer.height[0] = dims.height;
      pixel_buffer.stride[0] = 0; /* What should we store here? */
      pixel_buffer.nbytes = total_length;
      pixel_buffer.pixel_format = [self getCapturePixelFormat: pix_fmt];

      is_pixel_buffer_set = 1;
    }

    pixel_buffer.plane[0] = (uint8_t*)data_pointer;
    
    cb_frame(pixel_buffer);

    return;
  }

  /* ------------------------------------------------------------------------------------------ */
  /* Uncompressed Formats                                                                       */
  /* ------------------------------------------------------------------------------------------ */
  
  CVPixelBufferRef buffer = CMSampleBufferGetImageBuffer(sampleBuffer);  /* Note: CVImageBufferRef == CVPixelBufferRef, see: http://stackoverflow.com/questions/18660861/convert-cvimagebufferref-to-cvpixelbufferref */
  if (NULL == buffer) {
    CMFormatDescriptionRef desc = CMSampleBufferGetFormatDescription(sampleBuffer);
    CMVideoDimensions dims = CMVideoFormatDescriptionGetDimensions(desc);
    CMMediaType media_type = CMFormatDescriptionGetMediaType(desc);
    FourCharCode fcc = CMFormatDescriptionGetMediaSubType(desc);
    char* fcc_ptr = (char*)&fcc;
    printf("Error: the returned CVImageBufferRef is NULL. Stopping, sampleBuffer is %p.\n", sampleBuffer);
    printf("Error: CMSampleBufferGetSampleSize() = %lu\n", CMSampleBufferGetTotalSampleSize(sampleBuffer));
    printf("Error: CMVideoDimensions, width: %d, height: %d\n", dims.width, dims.height);
    printf("Error: CMMediaType, is video: %c\n", media_type == kCMMediaType_Video ? 'y' : 'c');
    printf("Error: FourCharCode: %c%c%c%c\n", fcc_ptr[3], fcc_ptr[2], fcc_ptr[1], fcc_ptr[1]);
    return;
  }

  /* Fill the pixel_buffer member with some info that won't change per frame. */
  if (0 == is_pixel_buffer_set) {
    
    CMFormatDescriptionRef desc = CMSampleBufferGetFormatDescription(sampleBuffer);
    CMPixelFormatType pix_fmt = CMFormatDescriptionGetMediaSubType(desc);
    
    if (true == CVPixelBufferIsPlanar(buffer)) {
      size_t plane_count = CVPixelBufferGetPlaneCount(buffer);
      if (plane_count > 3) {
        printf("Error: we got a plane count bigger then 3, not supported yet. Stopping.\n");
        exit(EXIT_FAILURE);
      }
      for (size_t i = 0; i < plane_count; ++i) {
        pixel_buffer.width[i] = CVPixelBufferGetWidthOfPlane(buffer, i);
        pixel_buffer.height[i] = CVPixelBufferGetHeightOfPlane(buffer, i);
        pixel_buffer.stride[i] = CVPixelBufferGetBytesPerRowOfPlane(buffer, i);
        pixel_buffer.nbytes += pixel_buffer.stride[i] * pixel_buffer.height[i];

        printf("width: %lu, height: %lu, stride: %lu, nbytes: %lu, plane_count: %lu\n", 
             pixel_buffer.width[i],
             pixel_buffer.height[i],
             pixel_buffer.stride[i],
             pixel_buffer.nbytes,
             plane_count);
      }
    }
    else {
      pixel_buffer.width[0] = CVPixelBufferGetWidth(buffer);
      pixel_buffer.height[0] = CVPixelBufferGetHeight(buffer);
      pixel_buffer.stride[0] = CVPixelBufferGetBytesPerRow(buffer);
      pixel_buffer.nbytes = pixel_buffer.stride[0] * pixel_buffer.height[0];
    }
    
    pixel_buffer.pixel_format = [self getCapturePixelFormat: pix_fmt];
    
    is_pixel_buffer_set = 1;
  }

  CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
  {
    if (CA_YUYV422 == pixel_format     /* kCVPixelFormatType_422YpCbCr8_yuvs */
        || CA_UYVY422 == pixel_format  /* kCVPixelFormatType_422YpCbCr8 */
        || CA_BGRA32 == pixel_format   /* kCVPixelFormatType_32BGRA */
    )
    {
      pixel_buffer.plane[0] = (uint8_t*)CVPixelBufferGetBaseAddress(buffer);
    }
    else if (CA_YUVJ420BP == pixel_format    /* kCVPixelFormatType_420YpCbCr8BiPlanarFullRange */
             || CA_YUV420BP == pixel_format) /* kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange */
    {
      pixel_buffer.plane[0] = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(buffer, 0);
      pixel_buffer.plane[1] = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(buffer, 1);
    }
    else {
      printf("Error: unhandled or unknown pixel format for the received buffer: %d.\n", pixel_format);
      CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
      return;
    }

    cb_frame(pixel_buffer);
  }
  
  CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);


  /* --------------------- OLD CODE LEAVING THIS HERE FOR REFERENCE -------------------------- */
#if 0
  CMFormatDescriptionRef desc_ref = CMSampleBufferGetFormatDescription(sampleBuffer);

  int pix_fmt = CMFormatDescriptionGetMediaSubType(desc_ref);
  if (kCVPixelFormatType_422YpCbCr8 == pix_fmt) { 
    printf("> kCVPixelFormatType_422YpCbCr8 (CA_UYVY422).\n");
  }
  else if(kCVPixelFormatType_422YpCbCr8_yuvs == pix_fmt) {
    printf("> kCVPixelFormatType_422YpCbCr8_yuvs (CA_YUYV422).\n");
  }
  else if (kCVPixelFormatType_32BGRA == pix_fmt) {
    printf("> kCVPixelFormatType_32BGRA.\n");
  }
#endif

#if 0
  CVImageBufferRef img_ref = CMSampleBufferGetImageBuffer(sampleBuffer);
  CVPixelBufferLockBaseAddress(img_ref, 0);
  void* base_address = CVPixelBufferGetBaseAddress(img_ref);

  // get number of bytes in the image.
  size_t img_bytes_per_row = CVPixelBufferGetBytesPerRow(img_ref);
  size_t img_height = CVPixelBufferGetHeight(img_ref);
  size_t nbytes = img_bytes_per_row * img_height;

  if(cb_frame) {
    cb_frame((void*)base_address, nbytes, cb_user);
  }

  CVPixelBufferUnlockBaseAddress(img_ref, 0);
#endif

#if 0
  // Some debug info.
  CMFormatDescriptionRef format_desc_ref = CMSampleBufferGetFormatDescription(sampleBuffer);
  FourCharCode active_video_type = CMFormatDescriptionGetMediaSubType(format_desc_ref);
  int av_fmt = CMFormatDescriptionGetMediaSubType(format_desc_ref);
  if(av_fmt == kCVPixelFormatType_422YpCbCr8) {
    printf("> kCVPixelFormatType_422YpCbCr8 (CA_UYVY422).\n");
  }
  else if(av_fmt == kCVPixelFormatType_422YpCbCr8_yuvs) {
    printf("> kCVPixelFormatType_422YpCbCr8_yuvs (CA_YUYV422).\n");
  }
  else if (av_fmt == kCVPixelFormatType_32BGRA) {
    printf("> kCVPixelFormatType_32BGRA.\n");
  }
#endif

}

  
// Get a list with all devices
- (int) getDevices: (std::vector<ca::Device>&) result {

  int index = 0;
  NSArray* devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];

  for(AVCaptureDevice* dev in devices) {
    ca::Device found_device;
    found_device.index = index;
    found_device.name = [[dev localizedName] UTF8String];
    result.push_back(found_device);
    ++index;
  }
    
  return (int) result.size();
}

// Get all the capabilities for the given device
- (int) getCapabilities: (std::vector<ca::Capability>&) result forDevice: (int) dev {

  AVCaptureDevice* d = [self getCaptureDevice: dev];
  if(d == nil) {
    return -1;
  }

  int fps_dx = 0;
  int fmt_dx = 0;

  for(AVCaptureDeviceFormat* f in [d formats]) {

    if(![[f mediaType] isEqualToString: AVMediaTypeVideo]) {
      fmt_dx++;
      continue;
    }
    
    CMFormatDescriptionRef fmt_description = [f formatDescription];
    CMPixelFormatType fmt_sub_type = CMFormatDescriptionGetMediaSubType(fmt_description);
    CMVideoDimensions dims = CMVideoFormatDescriptionGetDimensions([f formatDescription]);
    fps_dx = 0;

    for(AVFrameRateRange* fps in [f videoSupportedFrameRateRanges]) {

      ca::Capability cap;
      
      if ([fps minFrameRate] != [fps maxFrameRate]) {
#if !defined(NDEBUG)        
        printf("@todo -  Need to handle a capability with different min/max framerates. This works but need more testing. min framerate: %f, max framerate: %f\n", (float)[fps minFrameRate], (float)[fps maxFrameRate]);
#endif
        cap.fps = ca::fps_from_rational((uint64_t)1, (uint64_t) [fps maxFrameRate]);
      }
      else {
        CMTime dur = [fps maxFrameDuration];
        cap.fps = ca::fps_from_rational((uint64_t)dur.value, (uint64_t)dur.timescale);
      }

      cap.width = dims.width;
      cap.height = dims.height;
      cap.pixel_format = [self getCapturePixelFormat: fmt_sub_type];
      cap.user = (void*) f;
      cap.capability_index = (int) result.size();
      cap.fps_index = fps_dx;
      cap.pixel_format_index = fmt_dx;

      result.push_back(cap);

      ++fps_dx;	
    }

    fmt_dx++;
  }
 
  return (int) result.size();
}

// Get the formats into which we can convert the incoming data.
- (int) getOutputFormats: (std::vector<ca::Format>&) result {

  if(output == nil) {
    output = [[AVCaptureVideoDataOutput alloc] init];
    if(!output) {
      printf("Error: cannot allocate the video data output\n");
      return -1;
    }
    [output retain];
  }

  int c = 0;
  for(NSNumber* fmt in output.availableVideoCVPixelFormatTypes) {
    int ca_fmt = [self cvPixelFormatToCaptureFormat: fmt];
    ca::Format format;
    format.index = c;
    format.format = ca_fmt;
    result.push_back(format);
    c++;
  }

  /* Lists the video codecs */
#if 0
  NSArray* codec_types = output.availableVideoCodecTypes;
  for (NSString* fmt in codec_types) {
    NSLog(@"%@", fmt);
  }
#endif
  
  return (int) result.size();
}

/* Returns the output format that is used in the PixelBuffers that are passed into the callback. */
/*
- (int) getOutputFormat {
  return pixel_format;
}
*/

// Return the AVCaptureDevice* for the given index
- (AVCaptureDevice*) getCaptureDevice: (int) device {
  int c = 0;
  NSArray* devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
  for(AVCaptureDevice* dev in devices) {
    if(c == device) {
      return dev;
    }
    ++c;
  }
  return nil;
}

// Convert a pixel format type to a videocapture type
- (int) getCapturePixelFormat: (CMPixelFormatType) ref {

  switch(ref) {

    case kCVPixelFormatType_422YpCbCr8:                   { return CA_UYVY422;        } // Cb Y0 Cr Y1
    case kCVPixelFormatType_422YpCbCr8_yuvs:              { return CA_YUYV422;        } // Y0 Cb Y1 Cr  
    case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange: { return CA_YUV420BP;       }
    case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:  { return CA_YUVJ420BP;      }
    case kCVPixelFormatType_32ARGB:                       { return CA_ARGB32;         }
    case kCVPixelFormatType_32BGRA:                       { return CA_BGRA32;         }
    case kCMVideoCodecType_JPEG_OpenDML:                  { return CA_JPEG_OPENDML;   }

    default: {
      NSNumber* pixnum = [NSNumber numberWithInt: ref];
      std::string type = [self cvPixelFormatToString:pixnum];
      printf("Unknown and unhandled pixel format type: %u, %s\n", (unsigned int)ref, type.c_str());
      return CA_NONE;
    }
  };
}

/* Debugging & utils */
/* -------------------------------------- */

- (NSString* const) widthHeightToCaptureSessionPreset:(int) w andHeight:(int) h {
#if TARGET_OS_IPHONE
  if (w == 352 && h == 288) { return AVCaptureSessionPreset352x288; }  /* iPhone 5.0+ */
  else if (w == 640 && h == 480) { return AVCaptureSessionPreset640x480; } /* iPhone 4.0+ */
  else if (w == 1280 && h == 720) { return AVCaptureSessionPreset1280x720; } /* iPhone 4.0+ */
  else if (w == 1920 && h == 1080) { return AVCaptureSessionPreset1920x1080; } /* iPhone 5.0+ */
  else if (w == 960 && h == 540) { return AVCaptureSessionPresetiFrame960x540; } /* iPhone 5.0+ */
  else {
    return AVCaptureSessionPresetHigh;
  }
#else
  if(w == 320 && h == 240) { return AVCaptureSessionPreset320x240; }
  else if(w == 352 && h == 288) { return AVCaptureSessionPreset352x288; } 
  else if(w == 640 && h == 480) { return AVCaptureSessionPreset640x480; }
  else if(w == 960 && h == 540) { return AVCaptureSessionPreset960x540; }
  else if(w == 1280 && h == 720) { return AVCaptureSessionPreset1280x720; }
  else {
    return AVCaptureSessionPresetHigh; // if no preset matches we return the highest
  }
#endif
}

- (void) printSupportedPixelFormatsByVideoDataOutput: (AVCaptureVideoDataOutput*) o {
  for(NSNumber* fmt in o.availableVideoCVPixelFormatTypes) {
    std::string name = [self cvPixelFormatToString: fmt];
    printf("> %s\n", name.c_str());
  }
}

- (int) cvPixelFormatToCaptureFormat: (NSNumber*) fmt {

  int ifmt = [fmt integerValue];
  switch(ifmt) {
    case kCVPixelFormatType_422YpCbCr8:                    return CA_UYVY422; // Cb Y0 Cr Y1
    case kCVPixelFormatType_422YpCbCr8_yuvs:               return CA_YUYV422; // Y0 Cb Y1 Cr  
    case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:  return CA_YUV420BP;  
    case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:   return CA_YUVJ420BP;  
    case kCVPixelFormatType_32ARGB:                        return CA_ARGB32;  
    case kCVPixelFormatType_32BGRA:                        return CA_BGRA32;
    case kCMVideoCodecType_JPEG_OpenDML:                   return CA_JPEG_OPENDML;
    default:                                               return CA_NONE;
  }
}

- (int) captureFormatToCvPixelFormat: (int) fmt {
  switch(fmt) {
    case CA_UYVY422:        return kCVPixelFormatType_422YpCbCr8 ;
    case CA_YUYV422:        return kCVPixelFormatType_422YpCbCr8_yuvs;  
    case CA_YUV420BP:       return kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;  
    case CA_YUVJ420BP:      return kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;  
    case CA_ARGB32:         return kCVPixelFormatType_32ARGB;  
    case CA_BGRA32:         return kCVPixelFormatType_32BGRA;
    case CA_JPEG_OPENDML:   return kCMVideoCodecType_JPEG_OpenDML;
    default:                return CA_NONE;
  }
}

- (std::string) cvPixelFormatToString: (NSNumber*) fmt {
  NSDictionary *formats = [NSDictionary dictionaryWithObjectsAndKeys:
      @"kCVPixelFormatType_1Monochrome", [NSNumber numberWithInt:kCVPixelFormatType_1Monochrome],
      @"kCVPixelFormatType_2Indexed", [NSNumber numberWithInt:kCVPixelFormatType_2Indexed],
      @"kCVPixelFormatType_4Indexed", [NSNumber numberWithInt:kCVPixelFormatType_4Indexed],
      @"kCVPixelFormatType_8Indexed", [NSNumber numberWithInt:kCVPixelFormatType_8Indexed],
      @"kCVPixelFormatType_1IndexedGray_WhiteIsZero", [NSNumber numberWithInt:kCVPixelFormatType_1IndexedGray_WhiteIsZero],
      @"kCVPixelFormatType_2IndexedGray_WhiteIsZero", [NSNumber numberWithInt:kCVPixelFormatType_2IndexedGray_WhiteIsZero],
      @"kCVPixelFormatType_4IndexedGray_WhiteIsZero", [NSNumber numberWithInt:kCVPixelFormatType_4IndexedGray_WhiteIsZero],
      @"kCVPixelFormatType_8IndexedGray_WhiteIsZero", [NSNumber numberWithInt:kCVPixelFormatType_8IndexedGray_WhiteIsZero],
      @"kCVPixelFormatType_16BE555", [NSNumber numberWithInt:kCVPixelFormatType_16BE555],
      @"kCVPixelFormatType_16LE555", [NSNumber numberWithInt:kCVPixelFormatType_16LE555],
      @"kCVPixelFormatType_16LE5551", [NSNumber numberWithInt:kCVPixelFormatType_16LE5551],
      @"kCVPixelFormatType_16BE565", [NSNumber numberWithInt:kCVPixelFormatType_16BE565],
      @"kCVPixelFormatType_16LE565", [NSNumber numberWithInt:kCVPixelFormatType_16LE565],
      @"kCVPixelFormatType_24RGB", [NSNumber numberWithInt:kCVPixelFormatType_24RGB],
      @"kCVPixelFormatType_24BGR", [NSNumber numberWithInt:kCVPixelFormatType_24BGR],
      @"kCVPixelFormatType_32ARGB", [NSNumber numberWithInt:kCVPixelFormatType_32ARGB],
      @"kCVPixelFormatType_32BGRA", [NSNumber numberWithInt:kCVPixelFormatType_32BGRA],
      @"kCVPixelFormatType_32ABGR", [NSNumber numberWithInt:kCVPixelFormatType_32ABGR],
      @"kCVPixelFormatType_32RGBA", [NSNumber numberWithInt:kCVPixelFormatType_32RGBA],
      @"kCVPixelFormatType_64ARGB", [NSNumber numberWithInt:kCVPixelFormatType_64ARGB],
      @"kCVPixelFormatType_48RGB", [NSNumber numberWithInt:kCVPixelFormatType_48RGB],
      @"kCVPixelFormatType_32AlphaGray", [NSNumber numberWithInt:kCVPixelFormatType_32AlphaGray],
      @"kCVPixelFormatType_16Gray", [NSNumber numberWithInt:kCVPixelFormatType_16Gray],
      @"kCVPixelFormatType_422YpCbCr8", [NSNumber numberWithInt:kCVPixelFormatType_422YpCbCr8],
      @"kCVPixelFormatType_4444YpCbCrA8", [NSNumber numberWithInt:kCVPixelFormatType_4444YpCbCrA8],
      @"kCVPixelFormatType_4444YpCbCrA8R", [NSNumber numberWithInt:kCVPixelFormatType_4444YpCbCrA8R],
      @"kCVPixelFormatType_444YpCbCr8", [NSNumber numberWithInt:kCVPixelFormatType_444YpCbCr8],
      @"kCVPixelFormatType_422YpCbCr16", [NSNumber numberWithInt:kCVPixelFormatType_422YpCbCr16],
      @"kCVPixelFormatType_422YpCbCr10", [NSNumber numberWithInt:kCVPixelFormatType_422YpCbCr10],
      @"kCVPixelFormatType_444YpCbCr10", [NSNumber numberWithInt:kCVPixelFormatType_444YpCbCr10],
      @"kCVPixelFormatType_420YpCbCr8Planar", [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8Planar],
      @"kCVPixelFormatType_420YpCbCr8PlanarFullRange", [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8PlanarFullRange],
      @"kCVPixelFormatType_422YpCbCr_4A_8BiPlanar", [NSNumber numberWithInt:kCVPixelFormatType_422YpCbCr_4A_8BiPlanar],
      @"kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange", [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange],
      @"kCVPixelFormatType_420YpCbCr8BiPlanarFullRange", [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8BiPlanarFullRange],
      @"kCVPixelFormatType_422YpCbCr8_yuvs", [NSNumber numberWithInt:kCVPixelFormatType_422YpCbCr8_yuvs],
      @"kCVPixelFormatType_422YpCbCr8FullRange", [NSNumber numberWithInt:kCVPixelFormatType_422YpCbCr8FullRange],
      @"kCMVideoCodecType_422YpCbCr8", [NSNumber numberWithInt:kCMVideoCodecType_422YpCbCr8],
      @"kCMVideoCodecType_Animation", [NSNumber numberWithInt:kCMVideoCodecType_Animation],
      @"kCMVideoCodecType_Cinepak", [NSNumber numberWithInt:kCMVideoCodecType_Cinepak],
      @"kCMVideoCodecType_JPEG", [NSNumber numberWithInt: kCMVideoCodecType_JPEG],
      @"kCMVideoCodecType_JPEG_OpenDML", [NSNumber numberWithInt:kCMVideoCodecType_JPEG_OpenDML],
      @"kCMVideoCodecType_SorensonVideo", [NSNumber numberWithInt:kCMVideoCodecType_SorensonVideo],
      @"kCMVideoCodecType_SorensonVideo3", [NSNumber numberWithInt:kCMVideoCodecType_SorensonVideo3],
      @"kCMVideoCodecType_H263", [NSNumber numberWithInt:kCMVideoCodecType_H263],
      @"kCMVideoCodecType_H264", [NSNumber numberWithInt:kCMVideoCodecType_H264],
      @"kCMVideoCodecType_MPEG4Video", [NSNumber numberWithInt:kCMVideoCodecType_MPEG4Video],
      @"kCMVideoCodecType_MPEG2Video", [NSNumber numberWithInt:kCMVideoCodecType_MPEG2Video],
      @"kCMVideoCodecType_MPEG1Video", [NSNumber numberWithInt:kCMVideoCodecType_MPEG1Video],
      @"kCMVideoCodecType_DVCNTSC", [NSNumber numberWithInt:kCMVideoCodecType_DVCNTSC],
      @"kCMVideoCodecType_DVCPAL", [NSNumber numberWithInt:kCMVideoCodecType_DVCPAL],
      @"kCMVideoCodecType_DVCProPAL", [NSNumber numberWithInt:kCMVideoCodecType_DVCProPAL],
      @"kCMVideoCodecType_DVCPro50NTSC", [NSNumber numberWithInt:kCMVideoCodecType_DVCPro50NTSC],
      @"kCMVideoCodecType_DVCPro50PAL", [NSNumber numberWithInt:kCMVideoCodecType_DVCPro50PAL],
      @"kCMVideoCodecType_DVCPROHD720p60", [NSNumber numberWithInt:kCMVideoCodecType_DVCPROHD720p60],
      @"kCMVideoCodecType_DVCPROHD720p50", [NSNumber numberWithInt:kCMVideoCodecType_DVCPROHD720p50],
      @"kCMVideoCodecType_DVCPROHD1080i60", [NSNumber numberWithInt:kCMVideoCodecType_DVCPROHD1080i60],
      @"kCMVideoCodecType_DVCPROHD1080i50", [NSNumber numberWithInt:kCMVideoCodecType_DVCPROHD1080i50],
      @"kCMVideoCodecType_DVCPROHD1080p30", [NSNumber numberWithInt:kCMVideoCodecType_DVCPROHD1080p30],
      @"kCMVideoCodecType_DVCPROHD1080p25", [NSNumber numberWithInt:kCMVideoCodecType_DVCPROHD1080p25],
      @"kCMVideoCodecType_AppleProRes4444", [NSNumber numberWithInt:kCMVideoCodecType_AppleProRes4444],
      @"kCMVideoCodecType_AppleProRes422HQ", [NSNumber numberWithInt:kCMVideoCodecType_AppleProRes422HQ],
      @"kCMVideoCodecType_AppleProRes422", [NSNumber numberWithInt:kCMVideoCodecType_AppleProRes422],
      @"kCMVideoCodecType_AppleProRes422LT", [NSNumber numberWithInt:kCMVideoCodecType_AppleProRes422LT],
      @"kCMVideoCodecType_AppleProRes422Proxy", [NSNumber numberWithInt:kCMVideoCodecType_AppleProRes422Proxy],
      nil];

  NSString* name = [formats objectForKey: fmt];
  return [name UTF8String];
}

/* C-interface */
/* -------------------------------------- */

void* ca_av_alloc() {
  return (void*)[[AVFoundation_Implementation alloc] init];
}

void ca_av_dealloc(void* cap) {
  if(cap) {
    [(id)cap dealloc];
  }
}

int ca_av_get_devices(void* cap, std::vector<ca::Device>& result) {
  return [(id)cap getDevices:result];
}

int ca_av_get_capabilities(void* cap, int device, std::vector<ca::Capability>& result) {
  return [(id)cap getCapabilities:result forDevice:device];
}

int ca_av_get_output_formats(void* cap, std::vector<ca::Format>& result) {
  return [(id)cap getOutputFormats: result];
}

int ca_av_open(void* cap, ca::Settings settings) {
  return [(id)cap openDevice:settings];
}

int ca_av_close(void* cap) {
  return [(id)cap closeDevice];
}

int ca_av_start(void* cap) {
  return [(id)cap startCapture];
}

int ca_av_stop(void* cap) {
  return [(id)cap stopCapture];
}

void ca_av_set_callback(void* cap, ca::frame_callback fc, void* user) {
  [(id)cap setCallback: fc user:user];
}

@end

static void print_cmformatdescription_info(CMFormatDescriptionRef desc) {
  
  FourCharCode fcc = CMFormatDescriptionGetMediaSubType(desc);
  char* fcc_ptr = (char*)&fcc;
  CMVideoDimensions dims = CMVideoFormatDescriptionGetDimensions(desc);
  CMMediaType media_type = CMFormatDescriptionGetMediaType(desc);
  int pix_fmt = CMFormatDescriptionGetMediaSubType(desc);
    
  printf("Info: CMVideoDimensions, width: %d, height: %d\n", dims.width, dims.height);
  printf("Info: CMMediaType, is video: %c\n", media_type == kCMMediaType_Video ? 'y' : 'c');
  printf("Info: FourCharCode: %c%c%c%c\n", fcc_ptr[3], fcc_ptr[2], fcc_ptr[1], fcc_ptr[1]);

  if (kCVPixelFormatType_422YpCbCr8 == pix_fmt) { 
    printf("Info: MediaSubtype:  kCVPixelFormatType_422YpCbCr8 (CA_UYVY422).\n");
  }
  else if(kCVPixelFormatType_422YpCbCr8_yuvs == pix_fmt) {
    printf("Info: MediaSubtype: kCVPixelFormatType_422YpCbCr8_yuvs (CA_YUYV422).\n");
  }
  else if (kCVPixelFormatType_32BGRA == pix_fmt) {
    printf( "Info: MediaSubtype: kCVPixelFormatType_32BGRA.\n");
  }
  else if (kCMVideoCodecType_JPEG_OpenDML == pix_fmt) {
    printf("Info: MediaSubtype:  kCMVideoCodecType_JPEG_OpenDML.\n");
  }
  else {
    printf("Info: MediaSubtype:  Unknown: %d\n", pix_fmt);
  }
  printf("\n");
}
