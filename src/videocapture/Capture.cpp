#include <assert.h>
#include <videocapture/Capture.h>

namespace ca {

  Capture::Capture(frame_callback fc, void* user, int driver)
    :cap(NULL)
  {
    
#if defined(__APPLE__)
    if(cap == NULL && driver == CA_AV_FOUNDATION) {
      cap = new AVFoundation_Capture(fc, user);
    }
#endif

#if defined(__linux)
    if(cap == NULL && driver == CA_V4L2) {
      cap = new V4L2_Capture(fc, user);
    }
#endif

#if defined(_WIN32)
    if(cap == NULL && driver == CA_MEDIA_FOUNDATION) {
      cap = new MediaFoundation_Capture(fc, user);
    }
#endif

#if defined(USE_DECKLINK)
    if (cap != NULL && driver == CA_DECKLINK) {
      printf("Error: cap is already initialized but the driver is CA_DECKLINK! Not supposed to happen.\n");
      ::exit(EXIT_FAILURE);
    }
    
    if (driver == CA_DECKLINK) {
      cap = new Decklink(fc, user);
    }
#endif

    if(cap == NULL) {
      printf("Error: no valid capture driver found.\n");
      ::exit(EXIT_FAILURE);
    }
  }

  Capture::~Capture() {

    if(cap != NULL) {
      delete cap;
      cap = NULL;
    }
  }

  int Capture::open(Settings settings) {
    assert(cap != NULL);
    return cap->open(settings);
  }

  int Capture::close() {
    assert(cap != NULL);
    return cap->close();
  }

  int Capture::start() {
    assert(cap != NULL);
    return cap->start();
  }

  int Capture::stop() {
    assert(cap != NULL);
    return cap->stop();
  }

  void Capture::update() {
    assert(cap != NULL);
    cap->update();
  }

  std::vector<Capability> Capture::getCapabilities(int device) {
    assert(cap != NULL);
    return cap->getCapabilities(device);
  }

  std::vector<Device> Capture::getDevices() {
    assert(cap != NULL);
    return cap->getDevices();
  }

  std::vector<Format> Capture::getOutputFormats() {
    assert(cap != NULL);
    return cap->getOutputFormats();
  }

  int Capture::hasOutputFormat(int format) {
    assert(cap != NULL);
    
    std::vector<Format> formats = getOutputFormats();
    
    for (size_t i = 0; i < formats.size(); ++i) {
      if (formats[i].format == format) {
        return 0;
      }
    }
    
    return -1;
  }

  int Capture::listDevices() {
    assert(cap != NULL);
    return cap->listDevices();
  }
                        
  int Capture::listCapabilities(int device) {
    assert(cap != NULL);
    return cap->listCapabilities(device);
  }
 
  int Capture::listOutputFormats() {
    assert(cap != NULL);
    return cap->listOutputFormats();
  }

  int Capture::findCapability(int device, int width, int height, int fmt) {
    assert(cap != NULL);
    return cap->findCapability(device, width, height, fmt);
  }

  int Capture::findCapability(int device, std::vector<Capability> caps) {
    int capid = -1;
    for (size_t i = 0; i < caps.size(); ++i) {
      Capability& check = caps[i];
      printf("%d, %d\n", check.width, check.height);
      capid = cap->findCapability(device, check.width, check.height, check.pixel_format);
      if (capid >= 0) {
        break;
      }
    }
    return capid;
  }

  int Capture::findCapability(int device, int width, int height, int* fmts, int nfmts) {
    int capid = -1;
    for (int i = 0; i < nfmts; ++i) {
      capid = cap->findCapability(device, width, height, fmts[i]);
      if (capid >= 0) {
        return capid;
      }
    }
    return -1;
  }
  
  /* ------------------------------------------------------------------------- */

} // namespace ca
