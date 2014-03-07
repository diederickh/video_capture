#include <videocapture/mac/AVFoundation.h>
#include <stdlib.h>

namespace ca {
  
  AVFoundation::AVFoundation(frame_callback fc, void* user)
    :Base(fc, user)
    ,cap(NULL)
  {
    cap = ca_av_alloc();
    if(!cap) {
      printf("Error: cannot allocate the AVImplemenation.\n");
      ::exit(EXIT_FAILURE);
    }

    ca_av_set_callback(cap, fc, user);
  }

  AVFoundation::~AVFoundation() {

    close();

    if(cap) {
      ca_av_dealloc(cap);
      cap = NULL;
    }
    
  }

  int AVFoundation::open(Settings settings) {
    return ca_av_open(cap, settings);
  }

  int AVFoundation::close() {
    return ca_av_close(cap);
  }

  int AVFoundation::start() {
    return ca_av_start(cap);
  }

  int AVFoundation::stop() {
    return ca_av_stop(cap);
  }

  void AVFoundation::update() {
  }

  std::vector<Capability> AVFoundation::getCapabilities(int device) {
    std::vector<Capability> caps;
    ca_av_get_capabilities(cap, device, caps);
    return caps;
  }

  std::vector<Device> AVFoundation::getDevices() {
    std::vector<Device> result;
    ca_av_get_devices(cap, result);
    return result;
  }

  std::vector<Format> AVFoundation::getOutputFormats() {
    std::vector<Format> result;
    ca_av_get_output_formats(cap, result);
    return result;
  }

}; // namespace ca
