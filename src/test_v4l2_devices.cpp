/*

  Video 4 Linux Device Lists

        This was a basic test to get human readable names of capture
        devices using V4L2. We have support for udev, though udev is 
        not available on a couple of major districtions. Therefore when
        udev is not found we want use the features implemented here.
  
 */
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <map>
#include <string>
#include <dirent.h>
#include <unistd.h>                    /* readlink */
#include <string.h>                    /* memcmp */
#include <sys/stat.h>                  /* open */
#include <fcntl.h>                     /* open */
#include <stropts.h>                   /* ioctl */
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <algorithm>


static int list_devices();
static bool is_v4l2_dev(const char* name);

int main() {

  printf("\n\ntest_v4l2_devices.\n\n");

  list_devices();
  
  return 0;
}


static int list_devices() {

  printf("\nlist_devices.\n");

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
    return -1;
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
      printf("Warning: failed to open the device: %s\n", filename.c_str());
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

  /* Get readable descriptions. */
  for (size_t i = 0; i < files.size(); ++i) {

    std::string bus_info;
    std::string& filename = files[i];
    
    int fd = open(filename.c_str(), O_RDWR);
    if (fd < 0) {
      printf("Failed to open: %s", filename.c_str());
      continue;
    }

    ioctl(fd, VIDIOC_QUERYCAP, &vcap);
    close(fd);

    bus_info = (const char*) vcap.bus_info;
    
    if (true == cards[bus_info].empty()) {
      cards[bus_info] += std::string((char*)vcap.card) +" (" +bus_info +"):\n";
    }
    
    cards[bus_info] += "\t" +filename;

    if (false == links[filename].empty()) {
      cards[bus_info] += " <-" +links[filename];
    }

    cards[bus_info] += "\n";
  }

  {
    std::map<std::string, std::string>::iterator it = cards.begin();
    while (it != cards.end()) {
      printf("%s\n", it->second.c_str());
      ++it;
    }
  }

  return 0;
}

static bool is_v4l2_dev(const char* name) {

  if (NULL == name) {
    return false;
  }

  return (false == memcmp(name, "video", 5));
}
