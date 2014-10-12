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
    std::vector<Device> getDevices();
  };


} /* namespace ca */

#endif
