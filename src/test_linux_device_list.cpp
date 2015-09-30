#include <stdio.h>
#include <stdlib.h>
#include <videocapture/linux/V4L2_Devices.h>

using namespace ca;

int main() {

  printf("\n\nTest Devices.\n");

  std::vector<V4L2_Device> devices = v4l2_get_devices();

  for (size_t i = 0; i < devices.size(); ++i) {
    printf("> %s\n", devices[i].toString().c_str());
  }

  printf("\n\n");
  
  return 0;
}
