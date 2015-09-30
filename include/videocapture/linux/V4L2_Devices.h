/*

  Get device list
  ===============

  We abstract the way we retrieve devices on linux. Originally
  we used `udev` to retrieve a list of devices but this isn't 
  supported on all systems. Therefore we also implemented a 
  solution to retrieve devices which is more portatable 
  (see V4L2_Devices_Default.cpp).

 */
#ifndef VIDEO_CAPTURE_V4L2_DEVICES_UDEV_H
#define VIDEO_CAPTURE_V4L2_DEVICES_UDEV_H

extern "C" {
#  include <assert.h>
#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#  include <fcntl.h>
#  include <errno.h>
#  include <sys/mman.h>
#  include <sys/ioctl.h>
#  include <asm/types.h>
#  include <linux/videodev2.h>
#  include <locale.h>
#  include <unistd.h>
}

#include <videocapture/linux/V4L2_Types.h>

namespace ca {
  
  std::vector<V4L2_Device> v4l2_get_devices();
  
} /* namespace ca */

#endif
