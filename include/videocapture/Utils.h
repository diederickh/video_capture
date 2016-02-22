#ifndef VIDEO_CAPTURE_UTILS_H
#define VIDEO_CAPTURE_UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <videocapture/Types.h>

namespace ca {
  
  int fps_from_rational(uint64_t num, uint64_t den);          /* Converts a rational value to one of the CA_FPS_* values defined in Types.h */
  std::string format_to_string(int fmt);
  
}; // namespace ca

#endif
