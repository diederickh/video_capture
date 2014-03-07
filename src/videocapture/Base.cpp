#include <videocapture/Base.h>

namespace ca {

  Base::Base(frame_callback fc, void* user)
    :cb_frame(fc)
    ,cb_user(user)
  {
  }

  Base::~Base() {
  }

  // List all devices
  void Base::listDevices() {

    std::vector<Device> devices = getDevices();
    if(devices.size() == 0) {
      printf("No devices found");
      return;
    }

    for(size_t i = 0; i < devices.size(); ++i) {
      printf("%d) %s\n", devices[i].index, devices[i].name.c_str());
    }
  }

  // List the capabilities for the given device.
  void Base::listCapabilities(int device) {

    std::vector<Capability> caps = getCapabilities(device);
    for(size_t i = 0; i < caps.size(); ++i) {
      Capability& cb = caps[i];
      
      printf("%d) %d x %d @ %2.02f, %s\n", 
             cb.capability_index, 
             cb.width, 
             cb.height, 
             float(cb.fps/100.0f), 
             format_to_string(cb.pixel_format).c_str()
             
             );
    }
  }

  // List output formats.
  void Base::listOutputFormats() {
    std::vector<Format> ofmts = getOutputFormats();
    for(size_t i = 0; i < ofmts.size(); ++i) {
      printf("%d) %s\n", ofmts[i].index, format_to_string(ofmts[i].format).c_str());
    }
  }

}; // namespace ca
