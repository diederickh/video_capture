#include <videocapture/Types.h>

namespace ca { 

  /* CAPABILITY */
  /* -------------------------------------- */
  Capability::Capability() {
    clear();
  }

  Capability::~Capability() {
    clear();
  }

  void Capability::clear() {
    width = 0;
    height = 0;
    pixel_format = CA_NONE;
    fps = CA_NONE;
    capability_index = CA_NONE;
    fps_index = CA_NONE;
    pixel_format_index = CA_NONE;
    user = NULL;
  }

  /* DEVICE */
  /* -------------------------------------- */
  Device::Device() {
    clear();
  }

  Device::~Device() {
    clear();
  }

  void Device::clear() {
    index = -1;
    name.clear();
  }

  /* FORMAT */
  /* -------------------------------------- */
  Format::Format() {
    clear();
  }

  Format::~Format() {
    clear();
  }

  void Format::clear() {
    format = CA_NONE;
    index = CA_NONE;
  }

  /* Settings */
  /* -------------------------------------- */
  Settings::Settings() {
    clear();
  }

  Settings::~Settings() {
    clear();
  }

  void Settings::clear() {
    capability = CA_NONE;
    device = CA_NONE;
    format = CA_NONE;
  }

  /* Frame */
  /* -------------------------------------- */
  Frame::Frame() {
    clear();
  }

  Frame::~Frame() {
    clear();
  }

  void Frame::clear() {
    width.clear();
    height.clear();
    stride.clear();
    nbytes.clear();
  }

  int Frame::set(int w, int h, int fmt) {
    
    if(fmt == CA_YUYV422) {
      width.push_back(w / 2);
      height.push_back(h);
      stride.push_back(w / 2);
      nbytes.push_back(w * h * 2);
      return 1;
    }
    else {
      return -1;
    }
  }


} // namespace ca
