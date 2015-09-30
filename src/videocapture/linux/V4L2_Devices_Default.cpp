#include <vector>
#include <map>
#include <string>
#include <dirent.h>
#include <unistd.h>                                    /* readlink */
#include <string.h>                                    /* memcmp */
#include <sys/stat.h>                                  /* open */
#include <fcntl.h>                                     /* open */
#include <stropts.h>                                   /* ioctl */
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <algorithm>
#include <videocapture/linux/V4L2_Devices.h>

namespace ca {

  /*

    This file uses a "linux" standard way to retrieve capture 
    devices. Most of this code is based on v4l2-ctl which also 
    retrieves a list of devices. 

    This implementation can be used when udev is not available.
    
   */

  /* ---------------------------------------------------------------- */
  
  static bool is_v4l2_dev(const char* name);

  /* ---------------------------------------------------------------- */

  std::vector<V4L2_Device> v4l2_get_devices() {

    std::vector<V4L2_Device> result;    
    struct dirent* ep = NULL;
    struct v4l2_capability vcap;
    std::map<std::string, std::string> links;
    std::map<std::string, std::string> cards;
    std::vector<std::string> files;
    DIR* dp = NULL;

    /* Get /dev/video# devices. */
    dp = opendir("/dev");
    if (NULL == dp) {
      printf("Failed to open the /dev directory. Cannot list devices.\n");
      return result;
    }

    while (NULL != (ep = readdir(dp))) {
      if (true == is_v4l2_dev(ep->d_name)) {
        files.push_back(std::string("/dev/") +ep->d_name);
      }
    }

    if (0 != closedir(dp)) {
      printf("Failed to close the /dev dir handle.\n");
    }
  
    dp = NULL;

    /* Iterate over the found devices. */
    std::vector<std::string>::iterator it = files.begin();
    while (it != files.end()) {

      char link[64+1];
      int link_len;
      std::string target;
      std::string& filename = *it;

      link_len = readlink(filename.c_str(), link, 64);
      if (link_len < 0) {
        ++it;
        continue;
      }

      link[link_len] = '\0';

      if (link[0] != '/') {
        target = std::string("/dev/");
      }
    
      target += link;

      if (std::find(files.begin(), files.end(), target) == files.end()) {
        printf("File not found: %s\n", filename.c_str());
        ++it;
        continue;
      }

      if (links[target].empty()) {
        links[target] = filename;
      }
      else {
        links[target] += ", " +filename;
      }

      files.erase(it);
    }

    for (size_t i = 0; i < files.size(); ++i) {
      V4L2_Device device;
      device.path = files[i];
      result.push_back(device);
    }
    
    return result;
  }

  /* ---------------------------------------------------------------- */

  static bool is_v4l2_dev(const char* name) {

    if (NULL == name) {
      return false;
    }

    return (false == memcmp(name, "video", 5));
  }
  
} /* namespace ca */
