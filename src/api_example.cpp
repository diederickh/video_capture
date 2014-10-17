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
#include <fstream>
#include <videocapture/Capture.h>

#define WRITE_RAW_FILE 1

#if WRITE_RAW_FILE
bool wrote_frame = false;
std::fstream outfile;
#endif

using namespace ca;

bool must_run = true;

void fcallback(void* pixels, int nbytes, void* user);
void sig_handler(int sig);

int main() {
  printf("\nVideoCapture\n");

  signal(SIGINT, sig_handler);

#if WRITE_RAW_FILE
  outfile.open("generated.raw", std::ios::binary | std::ios::out);
  if (!outfile.is_open()) {
    printf("Error: failed to open `generated.raw`.\n");
    exit(1);
  }
#endif

  int width = 640;
  int height = 480;

  Settings cfg;
  cfg.device = 1;
  cfg.capability = 0;
  cfg.format = 0;

  Capture cap(fcallback, NULL); // , CA_DECKLINK);
  cap.listDevices();
  cap.listOutputFormats();
  cap.listCapabilities(cfg.device);

  return 0;

  std::vector<Capability> caps;
  //  caps.push_back(Capability(width, height, CA_YUYV422));
  caps.push_back(Capability(width, height, CA_UYVY422));
  //caps.push_back(Capability(width, height, CA_YUV420P));
  cfg.capability = cap.findCapability(cfg.device, caps);
  if (cfg.capability > 0) {
    printf("Found capability: %d\n", cfg.capability);
  }
  else {
    printf("Could not find any of the given capabilities.\n");
  }

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

#if WRITE_RAW_FILE
  outfile.flush();
  outfile.close();
  printf("Info: Wrote raw YUV file.\n");
#endif

  return EXIT_SUCCESS;
}

  void fcallback(void* pixels, int nbytes, void* user) {
  printf("Frame callback: %d bytes\n", nbytes);
#if WRITE_RAW_FILE
  if (false == wrote_frame) {
    outfile.write((const char*)pixels, nbytes);
    wrote_frame = true;
  }
#endif
}

void sig_handler(int sig) {
  printf("Handle signal.\n");
  must_run = false;
}
