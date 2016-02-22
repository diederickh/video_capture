#include <videocapture/Utils.h>

namespace ca {

  int fps_from_rational(uint64_t num, uint64_t den) {

    char buf[128] = { 0 } ;
    double r = 1.0 / ( double(num) / double(den) );
    float fps = 0.0f;

    sprintf(buf, "%2.02f", r);
    sscanf(buf, "%f", &fps);
    int v = (fps * 100);

    switch(v) {
      case CA_FPS_60_00: return CA_FPS_60_00;
      case CA_FPS_59_94: return CA_FPS_59_94;
      case CA_FPS_50_00: return CA_FPS_50_00;
      case CA_FPS_30_00: return CA_FPS_30_00;
      case CA_FPS_29_97: return CA_FPS_29_97;
      case CA_FPS_27_50: return CA_FPS_27_50;
      case CA_FPS_25_00: return CA_FPS_25_00;
      case CA_FPS_24_00: return CA_FPS_24_00;
      case CA_FPS_23_98: return CA_FPS_23_98;
      case CA_FPS_22_50: return CA_FPS_22_50;
      case CA_FPS_20_00: return CA_FPS_20_00;
      case CA_FPS_17_50: return CA_FPS_17_50;
      case CA_FPS_15_00: return CA_FPS_15_00;
      case CA_FPS_12_50: return CA_FPS_12_50;
      case CA_FPS_10_00: return CA_FPS_10_00;
      case CA_FPS_7_50:  return CA_FPS_7_50;
      case CA_FPS_5_00:  return CA_FPS_5_00;
      case CA_FPS_2_00:  return CA_FPS_2_00;
      default:           return CA_NONE;
    }
  }

  std::string format_to_string(int fmt) {
    switch(fmt) {
      case CA_UYVY422:          return "CA_UYVY422";
      case CA_YUYV422:          return "CA_YUYV422";
      case CA_YUV422P:          return "CA_YUV422P";
      case CA_YUV420P:          return "CA_YUV420P";
      case CA_YUV420BP:         return "CA_YUV420BP";
      case CA_YUVJ420P:         return "CA_YUVJ420P";
      case CA_YUVJ420BP:        return "CA_YUVJ420BP";
      case CA_ARGB32:           return "CA_ARGB32";
      case CA_BGRA32:           return "CA_BGRA32";
      case CA_RGB24:            return "CA_RGB24";
      case CA_JPEG_OPENDML:     return "CA_JPEG_OPENDML";
      case CA_H264:             return "CA_H264";
      case CA_MJPEG:            return "CA_MJPEG";
      case CA_NONE:             return "CA_NONE";
      default:                  return "UNKNOWN_FORMAT";
    }
  }

} // namespace ca
