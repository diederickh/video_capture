#ifndef VIDEO_CAPTURE_UTILS_H
#define VIDEO_CAPTURE_UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <videocapture/Types.h>

namespace ca {
  
  int fps_from_rational(uint64_t num, uint64_t den);          /* Converts a rational value to one of the CA_FPS_* values defined in Types.h */
  std::string format_to_string(int fmt);
  void yuv_to_rgb(uint8_t& y, uint8_t& u, uint8_t& v, double& r, double& g, double& b); /* see: https://msdn.microsoft.com/de-de/library/windows/desktop/dd206750%28v=vs.85%29.aspx#colorspaceconversions */
  
}; // namespace ca

#endif
