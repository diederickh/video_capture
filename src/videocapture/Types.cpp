#include <videocapture/Types.h>

namespace ca { 

  /* PIXELBUFFER */
  /* -------------------------------------- */
  PixelBuffer::PixelBuffer() {
    nbytes = 0;
    stride[0] = 0;
    stride[1] = 0;
    stride[2] = 0;
    plane[0] = NULL;
    plane[1] = NULL;
    plane[2] = NULL;
    width[0] = 0;
    width[1] = 0;
    width[2] = 0;
    height[0] = 0;
    height[1] = 0;
    height[2] = 0;
    user = NULL;
  }
   
  /* CAPABILITY */
  /* -------------------------------------- */
  Capability::Capability() {
    clear();
  }

  Capability::Capability(int w, int h, int pixfmt) {
    clear();
    width = w;
    height = h;
    pixel_format = pixfmt;
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
    offset.clear();
  }

  int Frame::set(int w, int h, int fmt) {

    clear();

    if(fmt == CA_YUYV422) {
      width.push_back(w / 2);
      height.push_back(h);
      stride.push_back(w / 2);
      nbytes.push_back(w * h * 2);
      offset.push_back(0);
      return 1;
    }
    else if (fmt == CA_UYVY422) {
      width.push_back(w / 2);
      height.push_back(h);
      stride.push_back(w / 2);
      nbytes.push_back(w * h * 2);
      offset.push_back(0);
      return 1;
    }
    else if(fmt == CA_YUV420P) {

      // Y-channel
      width.push_back(w);
      height.push_back(h);
      stride.push_back(w);
      nbytes.push_back(w * h);
      offset.push_back(0);

      // Y-channel
      width.push_back(w / 2);
      height.push_back(h / 2);
      stride.push_back(w / 2);
      nbytes.push_back( (w / 2) * (h / 2) );
      offset.push_back(nbytes[0]);

      // V-channel
      width.push_back(w / 2);
      height.push_back(h / 2);
      stride.push_back(w / 2);
      nbytes.push_back( (w / 2) * (h / 2) );
      offset.push_back(nbytes[0] + nbytes[1]);
      return 1;
    }
    
    return -1;
  }


} // namespace ca
