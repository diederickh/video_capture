/*

  AVFoundation_Capture
  -------------

  Grabbing on Mac using AVFoundation.

 */
#ifndef VIDEO_CAPTURE_AV_FOUNDATION_H
#define VIDEO_CAPTURE_AV_FOUNDATION_H

#include <videocapture/Base.h>
#include <videocapture/Types.h>
#include <videocapture/Utils.h>
#include <videocapture/mac/AVFoundation_Interface.h>

namespace ca {

  class AVFoundation_Capture : public Base {

  public:
    AVFoundation_Capture(frame_callback fc, void* user);
    ~AVFoundation_Capture();

    /* Interface */
    int open(Settings settings);
    int close();
    int start();
    int stop();
    void update();
    
    /* Capabilities */
    //    int getOutputFormat();
    std::vector<Capability> getCapabilities(int device);
    std::vector<Device> getDevices();
    std::vector<Format> getOutputFormats();

  private:
    void* cap;                                               /* The AVFoundation_Implementation interface */
  };

}; // namespace ca


#endif
