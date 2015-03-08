#include <videocapture/Types.h>

namespace ca { 

  /* PIXELBUFFER */
  /* -------------------------------------- */
  PixelBuffer::PixelBuffer() {
    pixels = NULL;
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
    offset[0] = 0;
    offset[1] = 0;
    offset[2] = 0;
    pixel_format = CA_NONE;
    user = NULL;
  }
   
  int PixelBuffer::setup(int w, int h, int fmt) {

    if (0 == w || 0 == h) {
      printf("error: cannot setup pixel buffer because w or h is 0.\n");
      return -1;
    }

    pixel_format = fmt;
    width[0] = w;
    height[0] = h;

    switch (fmt) {

      case CA_YUV420P: {
        stride[0] = w;
        stride[1] = w / 2;
        stride[2] = w / 2;

        width[0] = w;
        width[1] = w / 2;
        width[2] = w / 2;

        height[0] = h;
        height[1] = h / 2;
        height[2] = h / 2;

        offset[0] = 0;
        offset[1] = (size_t)(w * h);
        offset[2] = (size_t)(offset[1] + (w / 2) * (h / 2));

        break;
      }
        /*
      case CA_YUYV422: 
      case CA_UYVY422: {
        stride[0] = w;
        stride[1] = w / 2;
        stride[2] = w / 2;
        break;
      }
        */

      default: {
        printf("error: cannot setup the PixelBuffer for the given fmt: %d\n", fmt);
        return -2;
      }
    }

    return 0;
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
    filter_score = 0;
    index = -1;
  }

  /* CAPABILITY FILTER */
  /* -------------------------------------- */
  CapabilityFilter::CapabilityFilter(int attribute, double value, int priority)
    :attribute(attribute)
    ,value(value)
    ,priority(priority)
  {

  }

  CapabilityFilter::~CapabilityFilter() {
    clear();
  }

  void CapabilityFilter::clear() {
    attribute = CA_NONE;
    value = 0.0;
    priority = 0;
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
