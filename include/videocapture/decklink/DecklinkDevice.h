#ifndef VIDEO_CAPTURE_DECKLINK_DEVICE_H
#define VIDEO_CAPTURE_DECKLINK_DEVICE_H

#include <decklink/DecklinkAPI.h>

namespace ca { 

  class DecklinkDevice {
  public:
    DecklinkDevice(IDeckLink* device);
    ~DecklinkDevice();

  public:
    IDeckLink* device;
  };

} /* namespace ca */

#endif
