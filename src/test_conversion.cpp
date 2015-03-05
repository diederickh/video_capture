/*
  
  Test conversion
  ===============

  Some SDKs support automatic conversion of compressed input streams to 
  uncompressed output streams. For example the Logitech C920 camera 
  has support for JPEG and H264. By setting another value then `CA_NONE`
  we can use the SDK to convert/uncompress the incoming data. 

  This code was written and tested on Mac using AVFoundation using a 
  Logitech C920 webcam.

 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <videocapture/Capture.h>

using namespace ca;

static void fcallback(PixelBuffer& buffer); 

int main() {
  
  int r = 0;
  int formats[] = { CA_JPEG_OPENDML } ;
  int width = 1280;
  int height = 720;
  
  Settings cfg;
  cfg.device = 1;
  cfg.capability = 0;
  cfg.format = CA_UYVY422;

  Capture cap(fcallback, NULL);

  /* List the devices and output formats. */
  cap.listDevices();
  cap.listOutputFormats();

  /* Check if there the capability is supported. */
  cfg.capability = cap.findCapability(cfg.device, width, height, formats, 1);
  if (cfg.capability < 0) {
    cap.listCapabilities(cfg.device);
    printf("Error: failed to find the capability.\n");
    return 1;
  }

  printf("We found a capability for %d x %d, capability: %d\n", width, height, cfg.capability);

  /* Open the capture device. */
  r = cap.open(cfg);
  if (r < 0) {
    cap.listCapabilities(cfg.device);
    printf("Error: cannot open the device: %d\n", r);
    return 1;
  }

  /* Start capturing. */
  if (cap.start() < 0) {
    cap.listCapabilities(cfg.device);
    printf("Error: failed to start capturing.\n");
    return 1;
  }

  /* And start iterating. */
  while (true) {
    cap.update();
    usleep(5 * 1000);
  }
  
  return 0;
}


static void fcallback(PixelBuffer& buffer) {
  printf("Got a pixel buffer, size: %lu.\n", buffer.nbytes);

#if 0
  static int count = 0;
  static char fname[1024];
  
  sprintf(fname, "test_%04d.jpg", count);
  
  std::ofstream ofs(fname, std::ios::out | std::ios::binary);
  ofs.write((char*)buffer.plane[0], buffer.nbytes);
  ofs.close();

  count++;
#endif
}
