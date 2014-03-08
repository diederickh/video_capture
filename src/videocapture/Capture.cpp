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

  void Capture::listDevices() {
    assert(cap != NULL);
    cap->listDevices();
  }
                        
  void Capture::listCapabilities(int device) {
    assert(cap != NULL);
    cap->listCapabilities(device);
  }
 
  void Capture::listOutputFormats() {
    assert(cap != NULL);
    cap->listOutputFormats();
  }

} // namespace ca
