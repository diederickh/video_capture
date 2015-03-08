/*
 
  Cross Platform Interface
  ------------------------

  This class can be used to read back frames, cross  plaftorm, 
  from a capture device. By default we will select the media capture
  SDK for the current OS. Though you can specify what capture wrapper
  you want to use by passing one of the valid capture drivers to the
  constructor, see the list with supported drives in Types.h

  This class is a thin wrapper around the driver..

 */

#ifndef VIDEO_CAPTURE_VIDEOCAPTURE_H
#define VIDEO_CAPTURE_VIDEOCAPTURE_H

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <videocapture/Types.h>

#if defined(__APPLE__)
#  include <videocapture/mac/AVFoundation_Capture.h>
#elif defined(__linux)
#  include <videocapture/linux/V4L2_Capture.h>
#elif defined(_WIN32)
#  include <videocapture/win/MediaFoundation_Capture.h>
#endif

#if defined(USE_DECKLINK)
#  include <videocapture/decklink/Decklink.h>
#endif

namespace ca {

  class Capture {
  public:
    Capture(frame_callback fc, void* user, int driver = CA_DEFAULT_DRIVER);
    ~Capture();

    /* Interface */
    int open(Settings settings);
    int close();
    int start();
    int stop();
    void update();

    /* Capabilities */
    std::vector<Capability> getCapabilities(int device);
    std::vector<Device> getDevices();
    std::vector<Format> getOutputFormats();
    int hasOutputFormat(int format);
    
    /* Info */
    int listDevices();                         
    int listCapabilities(int device);          
    int listOutputFormats();                   
    int findCapability(int device, int width, int height, int fmt);
    int findCapability(int device, int width, int height, int* fmt, int nfmts);                 /* Test several different capture formats. */
    int findCapability(int device, std::vector<Capability> caps);                               /* Test the given capabilities in order and return the best one available. It wil the first found capability or -1 when none was found. */

  public:
    Base* cap;                                                                                  /* The capture implementation */
  };

} /* namespace ca */

#endif
