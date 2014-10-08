/*
  
  VideoCapture
  -------------

  This example shows a minimal example on how to list
  the capture devices, list capabilities and output formats.

 */
#include <signal.h>

#if defined(__APPLE__) || defined(__linux)
#  include <unistd.h> /* usleep */
#elif defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <videocapture/Capture.h>

using namespace ca;

bool must_run = true;

void fcallback(void* pixels, int nbytes, void* user);
void sig_handler(int sig);

int main() {
  printf("\nVideoCapture\n");

  signal(SIGINT, sig_handler);

  int width = 640;
  int height = 480;

  Settings cfg;
  cfg.device = 0;
  cfg.capability = 0;
  cfg.format = 0;

  Capture cap(fcallback, NULL);
  cap.listDevices();
  cap.listOutputFormats();
  cap.listCapabilities(cfg.device);

  int fmts[] = { CA_YUYV422, CA_UYVY422, CA_YUV420P }; 
  cfg.capability = cap.findCapability(cfg.device, width, height, fmts, 3);
  if (!cfg.capability) {
    printf("Error: tried CA_YUYV422 and CA_UYVY formats; both didn't work.");
    ::exit(EXIT_FAILURE);
  }

  if(cap.open(cfg) < 0) {
    printf("Error: cannot open the device.\n");
    ::exit(EXIT_FAILURE);
  }

  if(cap.start() < 0) {
    printf("Error: cannot start capture.\n");
    ::exit(EXIT_FAILURE);
  }

  while(must_run == true) {
    cap.update();
#if defined(_WIN32)
    Sleep(5);
#else
    usleep(5 * 1000);
#endif
  }

  if(cap.stop() < 0) {
    printf("Error: cannot stop.\n");
  }
  if(cap.close() < 0) {
    printf("Error: cannot close.\n");
  }

  return EXIT_SUCCESS;
}

void fcallback(void* pixels, int nbytes, void* user) {
  printf("Frame callback: %d bytes\n", nbytes);
}

void sig_handler(int sig) {
  printf("Handle signal.\n");
  must_run = false;
}
