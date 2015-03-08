#include <videocapture/mac/AVFoundation_Capture.h>
#include <stdlib.h>

namespace ca {
  
  AVFoundation_Capture::AVFoundation_Capture(frame_callback fc, void* user)
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

  AVFoundation_Capture::~AVFoundation_Capture() {

    close();

    if(cap) {
      ca_av_dealloc(cap);
      cap = NULL;
    }
  }

  int AVFoundation_Capture::open(Settings settings) {
    return ca_av_open(cap, settings);
}

  int AVFoundation_Capture::close() {
    return ca_av_close(cap);
  }

  int AVFoundation_Capture::start() {
    return ca_av_start(cap);
  }

  int AVFoundation_Capture::stop() {
    return ca_av_stop(cap);
  }

  void AVFoundation_Capture::update() {
  }

  std::vector<Capability> AVFoundation_Capture::getCapabilities(int device) {
    std::vector<Capability> caps;
    ca_av_get_capabilities(cap, device, caps);
    return caps;
  }

  std::vector<Device> AVFoundation_Capture::getDevices() {
    std::vector<Device> result;
    ca_av_get_devices(cap, result);
    return result;
  }

  std::vector<Format> AVFoundation_Capture::getOutputFormats() {
    std::vector<Format> result;
    ca_av_get_output_formats(cap, result);
    return result;
  }
  /*
  int AVFoundation_Capture::getOutputFormat() {
    return ca_av_get_output_format(cap);
  }
  */
  
}; // namespace ca
