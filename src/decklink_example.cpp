#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <videocapture/Capture.h>
#include <videocapture/decklink/Decklink.h>

using namespace ca;

static void on_signal(int s);
static void on_frame(void* pixels, int nbytes, void* user);

bool must_run = true;

int main() {

  printf("\n\nDecklink Example\n\n");

  signal(SIGINT, on_signal);

  Settings cfg;
  cfg.device = 0;
  cfg.capability = 15;
  
  ca::Decklink dl(on_frame, NULL);
  dl.listDevices();
  dl.getCapabilities(0);

  if (dl.open(cfg) < 0) {
    printf("Error: cannot open the decklink device.\n");
    exit(EXIT_FAILURE);
  };

  if (dl.start() < 0) {
    printf("Error: failed to start capturing from the device.\n");
    exit(EXIT_FAILURE);
  }

  while (must_run) {
    dl.update();
  }

  if (dl.stop() < 0) {
    printf("Error: something went wrong when trying to stop the capture.\n");
  }

  if (dl.close() < 0) {
    printf("Error: something went wrong when trying to close the capture device.\n");
  }

  return 0;
}

static void on_signal(int s) {
  static int called = 0;
  printf("Verbose: received a signal.\n");

  must_run = false;

  if (called == 1) {
    exit(1);
  }
  ++called;
}

static void on_frame(void* pixels, int nbytes, void* user) {
  printf("Received a frame of %d bytes.\n", nbytes);
}
