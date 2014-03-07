#include <signal.h>
#include <unistd.h> /* usleep */
#include <stdio.h>
#include <stdlib.h>

#if defined(__APPLE__)
#include <videocapture/mac/AVFoundation.h>
#elif defined(__linux)
#include <videocapture/linux/V4L2_Capture.h>
#endif

using namespace ca;

bool must_run = true;

void fcallback(void* pixels, int nbytes, void* user);
void sig_handler(int sig);

int main() {
  printf("\nVideoCapture\n");

  signal(SIGINT, sig_handler);

  Settings cfg;
  cfg.device = 0;
  cfg.capability = 212;
  cfg.format = 0;

#if defined(__APPLE__)
  AVFoundation cap(fcallback, NULL);
#elif defined(__linux)
  V4L2_Capture cap(fcallback, NULL);
#endif

  cap.listDevices();
  //cap.listOutputFormats();
  //cap.listCapabilities(0);

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
    usleep(5 * 1000);
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
