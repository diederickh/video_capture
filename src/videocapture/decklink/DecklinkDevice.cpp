#include <stdio.h>
#include <stdlib.h>
#include <videocapture/decklink/DecklinkDevice.h>

namespace ca {
  
  DecklinkDevice::DecklinkDevice(IDeckLink* device)
    :device(device)
  {
    if (NULL == device) {
      printf("Error: the IDeckLink device pointer is NULL. Not supposed to happen. Stopping now.\n");
      exit(EXIT_FAILURE);
    }

    device->AddRef();
  }

  DecklinkDevice::~DecklinkDevice() {

    if (device) {
      device->Release();
    }

    device = NULL;
  }
  
} /* namespace ca */
