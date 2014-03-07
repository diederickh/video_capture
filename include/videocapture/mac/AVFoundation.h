#ifndef VIDEO_CAPTURE_AV_FOUNDATION_H
#define VIDEO_CAPTURE_AV_FOUNDATION_H

#include <videocapture/Base.h>
#include <videocapture/Types.h>
#include <videocapture/Utils.h>
#include <videocapture/mac/AVInterface.h>

namespace ca {

  class AVFoundation : public Base {
  public:
    AVFoundation(frame_callback fc, void* user);
    ~AVFoundation();
    int open(Settings settings);
    int close();
    int start();
    int stop();
    void update();
    std::vector<Capability> getCapabilities(int device);
    std::vector<Device> getDevices();
    std::vector<Format> getOutputFormats();

  private:
    void* cap;                                               /* The AVImplementation interface */
  };

}; // namespace ca


#endif
