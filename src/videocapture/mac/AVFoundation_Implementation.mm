#import <videocapture/mac/AVFoundation_Implementation.h>

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
  }

  return self;
}

// Deallocate all retained objects
- (void) dealloc {

  [self closeDevice];

  cb_frame = nil;
  cb_user = nil;

  [super dealloc];
}

// Open a capture device
- (int) openDevice: (ca::Settings) settings {

  if(input != nil) {
    printf("Error: device already opened.\n");
    return -1;
  }
  
  // Get the capture device
  AVCaptureDevice* cap_device = [self getCaptureDevice: settings.device];
  if(cap_device == nil) {
    printf("Error: cannot find the given capture device: %d\n", settings.device);
    return -2;
  }

  // Get the input device
  NSError* err = nil;
  input = [AVCaptureDeviceInput deviceInputWithDevice: cap_device error:&err];
  if(input == nil) {
    printf("Error: cannot get the capture input device: %d\n", settings.device);
    return -3;
  }

  [input retain];

  // Create session
  session = [[AVCaptureSession alloc] init];
  if(session == nil) {
    printf("Error: cannot create the capture session.\n");
    return -4;
  }

  [session beginConfiguration];
  {

    // Get the capabilities for this device
    std::vector<ca::Capability> capabilities;
    [self getCapabilities: capabilities forDevice: settings.device];
    assert(capabilites.size() > 0);
    ca::Capability cap = capabilities[settings.capability];

    // Get the best matching session preset.
    NSString* const preset = [self widthHeightToCaptureSessionPreset: cap.width andHeight: cap.height];
    if([session canSetSessionPreset: preset]) {
      session.sessionPreset = preset;
    }
    else {
      printf("Error: cannot set the session preset.\n");
      return -6;
    }

    // Add the input
    if([session canAddInput: input] == NO) {
      printf("Error: cannot add the device input to the session manager.\n");
      return -5;
    }
    [session addInput:input];

    // Set the AVCaptureDeviceFormat and minFrameDuration
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

    // Create the output handler (needed for getOutputFormats)
    if(output == nil) {
      output = [[AVCaptureVideoDataOutput alloc] init];
      if(!output) {
        printf("Error: cannot allocate the video data output\n");
        return -9;
      }
      [output retain];
    }

    // Configure output
    CMPixelFormatType fmt_type = nil;
    std::vector<ca::Format> formats;
    [self getOutputFormats: formats];

    if(settings.format != CA_NONE) {
      assert(formats.size() > 0);
      fmt_type = [self captureFormatToCvPixelFormat: formats[settings.format].format];
    }
    else {
      fmt_type = [self captureFormatToCvPixelFormat: formats[0].format];
    }
    
    // Tell the AVCaptureVideoDataOutput to convert the incoming data if necessary (and specify w/h)
    [output setVideoSettings: [NSDictionary dictionaryWithObjectsAndKeys: 
                                 [NSNumber numberWithInt:fmt_type], kCVPixelBufferPixelFormatTypeKey,
                                 [NSNumber numberWithInteger:cap.width], (id)kCVPixelBufferWidthKey,
                                 [NSNumber numberWithInteger:cap.height], (id)kCVPixelBufferHeightKey,
                                 nil]];


    // Setup framegrabber callback
    dispatch_queue_t queue = dispatch_queue_create("Video Queue", DISPATCH_QUEUE_SERIAL);
    [output setSampleBufferDelegate:self queue:queue];

    if([session canAddOutput:output] == NO) {
      printf("Error: cannot add the given output.\n");
      dispatch_release(queue);
      [session commitConfiguration];
      return -10;
    }

    dispatch_release(queue);
    [session addOutput:output]; 
  }

  [session commitConfiguration];

  return 1;
}

// Close the device and shutdown the session.
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

  return 1;
}

// Start the capture process.
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

// Stop the capture process.
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

// Set the frame callback.
- (void) setCallback: (ca::frame_callback) cb user: (void*) u {
  cb_frame = cb;
  cb_user = u;
}

// Capture callback.
- (void) captureOutput: (AVCaptureOutput*) captureOutput
 didOutputSampleBuffer: (CMSampleBufferRef) sampleBuffer
        fromConnection: (AVCaptureConnection*) connection 
{
 
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

#if 0
  // Some debug info.
  /*
  CMFormatDescriptionRef format_desc_ref = CMSampleBufferGetFormatDescription(sampleBuffer);
  FourCharCode active_video_type = CMFormatDescriptionGetMediaSubType(format_desc_ref);
  int av_fmt = CMFormatDescriptionGetMediaSubType(format_desc_ref);
  int libav_fmt = [self avFoundationPixelFormatToLibavPixelFormat: av_fmt];
  std::string libav_fmt_str = rx_libav_pixel_format_to_string((AVPixelFormat)libav_fmt);
  NSString* av_fmt_str = [self getPixelFormatString:av_fmt];
  printf("Capturing in: %s, AVFoundation name: %s", libav_fmt_str.c_str(), [av_fmt_str UTF8String]);
  */
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
      
      if ([fps minFrameRate] != [fps maxFrameRate]) {
        printf("Need to handle a capability with different min/max framerates.\n");
      }
      else {
        ca::Capability cap;

        CMTime dur = [fps maxFrameDuration];
        cap.fps = ca::fps_from_rational((uint64_t)dur.value, (uint64_t)dur.timescale);
        cap.width = dims.width;
        cap.height = dims.height;
        cap.pixel_format = [self getCapturePixelFormat: fmt_sub_type];
        cap.user = (void*) f;
        cap.capability_index = (int) result.size();
        cap.fps_index = fps_dx;
        cap.pixel_format_index = fmt_dx;

        result.push_back(cap);
      }
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

  return (int) result.size();
}

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
    case kCMPixelFormat_422YpCbCr8:                return CA_UYVY422;
    case kCMPixelFormat_422YpCbCr8_yuvs:           return CA_YUYV422; 
    case kCMVideoCodecType_JPEG_OpenDML:           return CA_JPEG_OPENDML;
    default: return CA_NONE;
  };
}

/* Debugging & utils */
/* -------------------------------------- */

- (NSString* const) widthHeightToCaptureSessionPreset:(int) w andHeight:(int) h {

  if(w == 320 && h == 240) { return AVCaptureSessionPreset320x240; }
  else if(w == 352 && h == 288) { return AVCaptureSessionPreset352x288; } 
  else if(w == 640 && h == 480) { return AVCaptureSessionPreset640x480; }
  else if(w == 960 && h == 540) { return AVCaptureSessionPreset960x540; }
  else if(w == 1280 && h == 720) { return AVCaptureSessionPreset1280x720; }
  else {
    return AVCaptureSessionPresetHigh; // if no preset matches we return the highest
  }
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
    case kCVPixelFormatType_422YpCbCr8:                    return CA_UYVY422;
    case kCVPixelFormatType_422YpCbCr8_yuvs:               return CA_YUYV422;  
    case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:  return CA_YUV420BP;  
    case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:   return CA_YUVJ420BP;  
    case kCVPixelFormatType_32ARGB:                        return CA_ARGB32;  
    case kCVPixelFormatType_32BGRA:                        return CA_BGRA32;  
    default:                                               return CA_NONE;
  }
}

- (int) captureFormatToCvPixelFormat: (int) fmt {
  switch(fmt) {
    case CA_UYVY422:    return kCVPixelFormatType_422YpCbCr8 ;
    case CA_YUYV422:    return kCVPixelFormatType_422YpCbCr8_yuvs;  
    case CA_YUV420BP:   return kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;  
    case CA_YUVJ420BP:  return kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;  
    case CA_ARGB32:     return kCVPixelFormatType_32ARGB;  
    case CA_BGRA32:     return kCVPixelFormatType_32BGRA;  
    default:            return CA_NONE;
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
