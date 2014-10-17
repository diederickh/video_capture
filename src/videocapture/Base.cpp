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
  int Base::listDevices() {

    std::vector<Device> devices = getDevices();
    if(devices.size() == 0) {
      printf("No devices found");
      return -1;
    }

    for(size_t i = 0; i < devices.size(); ++i) {
      printf("[%d] %s\n", devices[i].index, devices[i].name.c_str());
    }

    return (int)devices.size();
  }

  // List the capabilities for the given device.
  int Base::listCapabilities(int device) {

    std::vector<Capability> caps = getCapabilities(device);
    if(caps.size() == 0) {
      return -1;
    }

    for(size_t i = 0; i < caps.size(); ++i) {
      Capability& cb = caps[i];
      
      printf("[%02d] %d x %d @ %2.02f, %s", 
             cb.capability_index, 
             cb.width, 
             cb.height, 
             float(cb.fps/100.0f), 
             format_to_string(cb.pixel_format).c_str()
             );

      if (cb.description.size() > 0) {
        printf(", %s", cb.description.c_str());
      }

      printf("\n");
    }
    return (int)caps.size();
  }

  // List output formats.
  int Base::listOutputFormats() {

    std::vector<Format> ofmts = getOutputFormats();
    if(ofmts.size() == 0) {
      return -1;
    }
    
    for(size_t i = 0; i < ofmts.size(); ++i) {
      printf("[%d] %s\n", ofmts[i].index, format_to_string(ofmts[i].format).c_str());
    }

    return (int)ofmts.size();
  }
  
  // Find a capability
  int Base::findCapability(int device, int width, int height, int fmt) {

    int fps = -1;
    int result = -1;
    std::vector<Capability> caps = getCapabilities(device);

    for(size_t i = 0; i < caps.size(); ++i) {
      Capability& cap = caps[i];
      if(cap.width == width 
         && cap.height == height 
         && cap.pixel_format == fmt
         && cap.fps > fps
         )
        {
          result = i;
          fps = cap.fps;
      }
    }

    return result;
  }

}; // namespace ca
