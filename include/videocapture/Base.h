 /*

  Base Video Capture 
  ------------------

  This base video capture class is an interface for platform specific implementations.
  Capabilities describe the features of a capture device, like width/height/fps/pixel_format.
  Output Formats describe any build in pixel/codec conversion methods a SDK/OS has (listed with fastest first).

 */

#ifndef VIDEO_CAPTURE_BASE_H
#define VIDEO_CAPTURE_BASE_H

#include <stdio.h>
#include <string>
#include <vector>
#include <videocapture/Types.h>
#include <videocapture/Utils.h>

namespace ca {
  
  class Base {
  public:
    Base(frame_callback fc, void* user);                                          /* Constructor, set the frame callback function which receives the given user pointer. */
    virtual ~Base();                                                              /* Destructor. */
    virtual int open(Settings cfg) = 0;                                           /* Open the device for the given settings object (with device, capability and output format). */
    virtual int close() = 0;                                                      /* Close the opened device. */
    virtual int start() = 0;                                                      /* Start captureing from the device. */
    virtual int stop() = 0;                                                       /* Stop captureing from the device. */
    virtual void update() = 0;                                                    /* Update, some implementations need to regurlarly update. Call this at the set FPS speed. */
    virtual std::vector<Capability> getCapabilities(int device) = 0;              /* Retrieve a list with capabilities. */
    virtual std::vector<Device> getDevices() = 0;                                 /* Retrieve a list with devices. */
    virtual std::vector<Format> getOutputFormats() = 0;                           /* Some capture SDKs have support for automatic conversion of the raw data it receives from capture devices to more common output values like YUV. */
    //    virtual int getOutputFormat() = 0;                                            /* This function should return the capture format that is used and by the capture SDK. This should be the final output format that is used. e.g. on Mac you can automotically convert from JPEG to a YUV* format, this function should return the YUV format. */  
    
    int listDevices();                                                           /* List the available capture devices for the implementation and return the number of found devices. */
    int listCapabilities(int device);                                            /* List the available capabilities for the given device. */
    int listOutputFormats();                                                     /* List the available output formats the at SDK of the OS/.. supports. On mac these are the output formats of the AVCaptureVideoDataOutput */
    int findCapability(int device, int width, int height, int fmt);              /* Get the best matching capability for the given format and dimensions. We return the capability index or -1 if not found. */

  public:
    frame_callback cb_frame;                                                      /* The frame callback. */
    void* cb_user;                                                                /* The user pointer that is passed into the frame callback. */
  };

}; // namespace ca

#endif
