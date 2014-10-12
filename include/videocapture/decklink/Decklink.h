#ifndef VIDEO_CAPTURE_DECKLINK_H
#define VIDEO_CAPTURE_DECKLINK_H

#include <string>
#include <vector>
#include <videocapture/Base.h>
#include <videocapture/Types.h>
#include <videocapture/Utils.h>
#include <decklink/DeckLinkAPI.h>

namespace ca {

  class Decklink {
  public:
    Decklink();
    ~Decklink();
    int listDevices();
    int listCapabilities();

    std::vector<Device> getDevices();
    std::vector<Capability> getCapabilities(int device);
    IDeckLink* getDevice(int index);

  public:

#if defined(_WIN32)
    static bool is_com_initialized;
#endif

  };


} /* namespace ca */

#endif
