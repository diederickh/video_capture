/*

  AVFoundation_Implementation
  ----------------

  This class implements a video capturer for Mac using AVFoundation. The 
  AVFoundation works by creating a session manager to which we add an input
  device. The input device contains a list of capabilities. These capabilities
  are represented using width/height/fps/pixel_format etc.. The VideoCapture library
  allows you to select a specific video capture capability. 

  Though on Mac there is another layer we need to take of. Besides the input devices
  (the webcam) it has a class for output data. This output data class can convert the
  raw incoming frames into a different pixel format. For example, when the input data
  from the webcam are JPEG frames, then the output object (AVCaptureVideoDataOutput)
  can convert this into YUV422 for you. 

 */
#include <TargetConditionals.h>
#include <videocapture/Types.h>
#include <videocapture/Utils.h>
#include <vector>

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>


// resource: https://webrtc.googlecode.com/svn/trunk/webrtc/modules/video_capture/ios/video_capture_ios_objc.mm
// resource: https://developer.apple.com/library/ios/documentation/AudioVideo/Conceptual/AVFoundationPG/AVFoundationPG.pdf

@interface AVFoundation_Implementation : NSObject<AVCaptureVideoDataOutputSampleBufferDelegate>
{
  AVCaptureSession* session;                                                                  /* Manages the state of the input device. */
  AVCaptureDeviceInput* input;                                                                /* Concrete instance of `AVDeviceInput`,  represents the input device (webcam). */
  AVCaptureVideoDataOutput* output;                                                           /* Concrete instance of `AVDeviceOutput`, used to get the video frames. */
  int pixel_format;                                                                           /* The pixel format in which we're capturing, is used in the callback to fill the PixelBuffer. This is a VideoCapture pixel format as defined in Types.h */
  int is_pixel_buffer_set;                                                                    /* Some information of the `pixel_buffer` member can only be set in the frame callback, but we don't want to set it every time we get a new frame, this flag is used for that. */
  ca::PixelBuffer pixel_buffer;                                                               /* The pixel buffer that is filled/set and pass to the capture callback. */
  ca::frame_callback cb_frame;                                                                /* Gets called when we receive a new frame. */
  void* cb_user;                                                                              /* User data that's will be passed into `cb_frame()` */
}

- (id) init;                                                                                   /* Initialize the AVImplementation object. */
- (void) dealloc;                                                                              /* Deallocate all objects that we retained. */
- (int) openDevice: (ca::Settings)settings;                                                    /* Opens the given device and sets the given capability. When settings.format is not CA_NONE, then we will ask the AVCaptureVideoDataOutput to use the given output (if supported). This will convert the data for you. */
- (int) closeDevice;                                                                           /* Closes and shutsdown the currently opened device */
- (int) startCapture;                                                                          /* Start the capture process on the opened device */
- (int) stopCapture;                                                                           /* Stop the capture process */
- (void) setCallback: (ca::frame_callback) cb user: (void*) u;                                 /* Set the callback function that will receive the frames */
- (int) getDevices: (std::vector<ca::Device>&) result;                                         /* Get a list with all the found capture devices. */
- (int) getCapabilities: (std::vector<ca::Capability>&) result forDevice: (int) dev;           /* Get the capabilities for the given device. */
- (int) getOutputFormats: (std::vector<ca::Format>&) result;                                   /* Get the output formats into which we can automaticaly convert the raw frames we receive from the webcam */
//- (int) getOutputFormat;                                                                       /* Get the output format that is used. The captured buffers are using this format. */
- (AVCaptureDevice*) getCaptureDevice: (int) device;                                           /* Get a specific capture device (represents a webcam for example). */
- (int) getCapturePixelFormat: (CMPixelFormatType) ref;                                        /* Converts the given CMPixelFormat to one which is used by the VideoCapture library */
- (void) printSupportedPixelFormatsByVideoDataOutput: (AVCaptureVideoDataOutput*) o;           /* Used for debugging. Shows a list of supported PixelFormats by the output object */
- (std::string) cvPixelFormatToString: (NSNumber*) fmt;                                        /* Used for debugging. Converts the given pixel format to a string. */
- (int) cvPixelFormatToCaptureFormat: (NSNumber*) fmt;                                         /* Converts the given pixel format, from the AVCaptureVideoDataOutput.availableVideoCVPixelFormatTypes, into the values we need to he video capture library */
- (int) captureFormatToCvPixelFormat: (int) fmt;                                               /* Converts one of the CA_RGB, CA_YUV* etc.. formats to one of the kCMPixelFormat* types */
- (NSString* const) widthHeightToCaptureSessionPreset: (int) w andHeight: (int) h;             /* Get the best matching session preset we set to the session manager */
- (void) captureOutput: (AVCaptureOutput*) captureOutput didOutputSampleBuffer: (CMSampleBufferRef) sampleBuffer fromConnection: (AVCaptureConnection*) connection; /* Capture callback implementation */

@end
