#ifndef VIDEO_CAPTURE_V4L2_UTILS_H
#define VIDEO_CAPTURE_V4L2_UTILS_H

extern "C" {
#  include <linux/videodev2.h>
}

namespace ca {

  int capture_format_to_v4l2_pixel_format(int fmt);
  int v4l2_pixel_format_to_capture_format(int fmt);
  std::string v4l2_pixel_format_to_string(int fmt);

} /* namespace ca */

#endif
