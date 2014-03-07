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

} // namespace ca
