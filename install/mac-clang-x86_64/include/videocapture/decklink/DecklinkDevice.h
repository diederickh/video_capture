#ifndef VIDEO_CAPTURE_DECKLINK_DEVICE_H
#define VIDEO_CAPTURE_DECKLINK_DEVICE_H

#include <stdio.h>
#include <videocapture/Base.h>
#include <videocapture/Types.h>
#include <videocapture/Utils.h>
#include <videocapture/decklink/DecklinkCallback.h>
#include <decklink/DecklinkAPI.h>

namespace ca { 

  class DecklinkDevice {
  public:
    DecklinkDevice(IDeckLink* device);
    ~DecklinkDevice();

    int open(Settings settings);
    int close();
    int start();
    int stop();
    void update();
    void setCallback(frame_callback cb, void* user);

  public:
    IDeckLink* device;
    IDeckLinkInput* input;
    DecklinkCallback* callback;

    BMDDisplayMode display_mode; /* The display mode we're going to use, is set in DecklinkDevice::open(). */
    BMDPixelFormat pixel_format; /* The pixel format we're going to use, is set in DecklinkDevice::open(). */
    bool is_started;             /* Is set to true in start(), and false in stop(). Just a safety check. */

    frame_callback fc;           /* Will be called when we received a video frame (may happen from a different thread. */ 
    void* user;                  /* Is passed into the frame callback. */
  };

  /* ------------------------------------------------------------------------- */

  inline void DecklinkDevice::setCallback(frame_callback cb, void* usr) {

    if(NULL == cb) {
      printf("Error: trying to set a callback but a NULL callback is given.\n");
      return;
    }
    
    fc = cb;
    user = usr;
  }

} /* namespace ca */

#endif


