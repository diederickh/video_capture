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

      /* Planar Formats */
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

      /* Bi Planar Formats: Plane 1: Y ; Plane 2: Packed U/V */
      case CA_YUV420BP: { // see: http://i.stack.imgur.com/Kyknv.png
        stride[0] = w;
        stride[1] = w;

        width[0] = w;
        width[1] = w / 2;

        height[0] = h;
        height[1] = h / 2;

        offset[0] = 0;
        offset[1] = (size_t)(w * h);

        break;
      }

      /* Packed Formats */
      case CA_YUYV422: 
      case CA_UYVY422: {
        stride[0] = w * 2;

        break;
      }

      case CA_RGB24:
      case CA_BGR24: {
        stride[0] = w * 3;

        break;
      }

      default: {
        printf("error: cannot setup the PixelBuffer for the given fmt: %d\n", fmt);
        return -2;
      }
    }

    return 0;
  }

  int PixelBuffer::get_rgb_pixels(uint8_t* rgb_pixels) {
    if (0 == nbytes) {
      printf("error: cannot return rgb pixels because pixel buffer is empty.\n");
      return -1;
    }

    switch (pixel_format) {

      /* Planar Formats */
      case CA_YUV420P: {
        uint8_t y, cb, cr;
        double r, g, b;
        size_t uv_index, y_index, rgb_index;
        size_t uv_width = width[1];
        size_t uv_height = height[1];
        size_t image_width = width[0];
        size_t image_height = height[0];

        for (int y_pos_uv = 0; y_pos_uv < uv_height; ++y_pos_uv) {
          for (int x_pos_uv = 0; x_pos_uv < uv_width; ++x_pos_uv) {
            uv_index = y_pos_uv * uv_width + x_pos_uv;
            cb = plane[1][uv_index];
            cr = plane[2][uv_index];

            // 1. Pixel
            y_index = (y_pos_uv * 2) * image_width + (x_pos_uv * 2);

            y = plane[0][y_index]; 
            yuv_to_rgb(y, cb, cr, r, g, b);
            rgb_index = 3 * y_index;
            rgb_pixels[rgb_index] = (uint8_t)r;
            rgb_pixels[rgb_index + 1] = (uint8_t)g;
            rgb_pixels[rgb_index + 2] = (uint8_t)b;

            // 2. Pixel
            ++y_index;

            y = plane[0][y_index];
            yuv_to_rgb(y, cb, cr, r, g, b);
            rgb_index = 3 * y_index;
            rgb_pixels[rgb_index] = (uint8_t)r;
            rgb_pixels[rgb_index + 1] = (uint8_t)g;
            rgb_pixels[rgb_index + 2] = (uint8_t)b;

            // 3. Pixel
            y_index = ((y_pos_uv * 2) + 1) * image_width + (x_pos_uv * 2);

            y = plane[0][y_index];
            yuv_to_rgb(y, cb, cr, r, g, b);
            rgb_index = 3 * y_index;
            rgb_pixels[rgb_index] = (uint8_t)r;
            rgb_pixels[rgb_index + 1] = (uint8_t)g;
            rgb_pixels[rgb_index + 2] = (uint8_t)b;

            // 4. Pixel
            ++y_index;

            y = plane[0][y_index];
            yuv_to_rgb(y, cb, cr, r, g, b);
            rgb_index = 3 * y_index;
            rgb_pixels[rgb_index] = (uint8_t)r;
            rgb_pixels[rgb_index + 1] = (uint8_t)g;
            rgb_pixels[rgb_index + 2] = (uint8_t)b;
          }
        }

        break;
      }

      /* Bi Planar Formats: Plane 1: Y ; Plane 2: Packed U/V */
      case CA_YUV420BP: { // see: http://i.stack.imgur.com/Kyknv.png
        uint8_t y, cb, cr;
        double r, g, b;
        size_t uv_index, y_index, rgb_index;
        size_t image_width = width[0];
        size_t image_height = height[0];
        size_t uv_width_times_2 = width[1]*2;
        size_t uv_height = height[1];

        for (int y_pos_uv = 0; y_pos_uv < uv_height; ++y_pos_uv) {
          for (int x_pos_uv = 0; x_pos_uv < uv_width_times_2; x_pos_uv += 2) { // += 2: UV packed in one plane
            uv_index = y_pos_uv * uv_width_times_2 + x_pos_uv;
            cb = plane[1][uv_index];
            cr = plane[1][uv_index + 1];

            // 1. Pixel
            y_index = (y_pos_uv * 2) * image_width + x_pos_uv;

            y = plane[0][y_index];
            yuv_to_rgb(y, cb, cr, r, g, b);
            rgb_index = 3 * y_index;
            rgb_pixels[rgb_index] = (uint8_t)r;
            rgb_pixels[rgb_index + 1] = (uint8_t)g;
            rgb_pixels[rgb_index + 2] = (uint8_t)b;

            // 2. Pixel
            ++y_index;

            y = plane[0][y_index];
            yuv_to_rgb(y, cb, cr, r, g, b);
            rgb_index = 3 * y_index;
            rgb_pixels[rgb_index] = (uint8_t)r;
            rgb_pixels[rgb_index + 1] = (uint8_t)g;
            rgb_pixels[rgb_index + 2] = (uint8_t)b;

            // 3. Pixel
            y_index = ((y_pos_uv * 2) + 1) * image_width + x_pos_uv;

            y = plane[0][y_index];
            yuv_to_rgb(y, cb, cr, r, g, b);
            rgb_index = 3 * y_index;
            rgb_pixels[rgb_index] = (uint8_t)r;
            rgb_pixels[rgb_index + 1] = (uint8_t)g;
            rgb_pixels[rgb_index + 2] = (uint8_t)b;

            // 4. Pixel
            ++y_index;

            y = plane[0][y_index];
            yuv_to_rgb(y, cb, cr, r, g, b);
            rgb_index = 3 * y_index;
            rgb_pixels[rgb_index] = (uint8_t)r;
            rgb_pixels[rgb_index + 1] = (uint8_t)g;
            rgb_pixels[rgb_index + 2] = (uint8_t)b;
          }
        }
        
        break;
      }

      /* Packed Formats */
      case CA_YUYV422: {  
        uint8_t* yuyv_image = plane[0];

        uint8_t y, cb, cr;
        double r, g, b;
        size_t image_width = width[0];
        size_t image_height = height[0];

        // Megapixel steps
        for (int i = 0, j = 0; i < image_width * image_height * 3; i += 6, j += 4) { // i: RGB steps ; j: YUYV steps
          cb = yuyv_image[j + 1];
          cr = yuyv_image[j + 3];

          // 1. Pixel
          y = yuyv_image[j];
          
          yuv_to_rgb(y, cb, cr, r, g, b);
          rgb_pixels[i] = (uint8_t)r;
          rgb_pixels[i + 1] = (uint8_t)g;
          rgb_pixels[i + 2] = (uint8_t)b;

          // 2. Pixel
          y = yuyv_image[j + 2];

          yuv_to_rgb(y, cb, cr, r, g, b);
          rgb_pixels[i + 3] = (uint8_t)r;
          rgb_pixels[i + 4] = (uint8_t)g;
          rgb_pixels[i + 5] = (uint8_t)b;
        }

        break;
      }

      /* TODO: Test with camera supporting this format whether it requires a vertical flip
      case CA_RGB24: {
        for (int i = 0; i < width[0] * height[0] * 3; ++i) {
          rgb_pixels[i] = plane[0][i];
        }

        break;
      }
      */

      case CA_BGR24: {
        size_t image_width = width[0];
        size_t image_height = height[0];
        size_t rgb_index_orig, rgb_index_new;

        // vertical flip + exchange R <-> B
        for (int y = 0; y < image_height; ++y) {
          for (int x = 0; x < image_width; ++x) {
            rgb_index_orig = 3 * (y * image_width + x);
            rgb_index_new = 3 * ((image_height - y) * image_width + x);

            rgb_pixels[rgb_index_new] = plane[0][rgb_index_orig + 2];
            rgb_pixels[rgb_index_new + 1] = plane[0][rgb_index_orig + 1];
            rgb_pixels[rgb_index_new + 2] = plane[0][rgb_index_orig];
          }
        }

        break;
      }

      default: {
        printf("error: cannot return rgb pixels because conversion is not defined for pixel_format: %d\n", pixel_format);
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
