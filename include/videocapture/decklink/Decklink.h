#ifndef VIDEO_CAPTURE_DECKLINK_H
#define VIDEO_CAPTURE_DECKLINK_H

#include <string>
#include <vector>
#include <videocapture/Base.h>
#include <videocapture/Types.h>
#include <videocapture/Utils.h>
#include <videocapture/decklink/DecklinkDevice.h>
#include <decklink/DeckLinkAPI.h>

namespace ca {

  class Decklink : public Base {
  public:
    Decklink(frame_callback fc, void* user);
    ~Decklink();

    int open(Settings settings);
    int close();
    int start();
    int stop();
    void update();

    //    int listDevices();
    //    int listCapabilities();

    std::vector<Device> getDevices();
    std::vector<Capability> getCapabilities(int device);
    std::vector<Format> getOutputFormats(); 

    IDeckLink* getDevice(int index);

  public:

#if defined(_WIN32)
    static bool is_com_initialized;
#endif

    DecklinkDevice* decklink_device;   /* The opened device. */
  };

} /* namespace ca */

#endif




