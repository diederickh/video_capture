/*
  
  MediaFoundation_Capture
  -----------------------

  Implements the Windows Media Foundation capture pipeline to read
  frames from capture devices newer Windows PCs (Win7, Win8). If you 
  want to use this class make sure that you've installed the Windows
  SDK.

  - See: http://msdn.microsoft.com/en-us/library/windows/desktop/ms694197(v=vs.85).aspx
  - Dependencies: Win SDK

 */
#ifndef VIDEO_CAPTURE_MEDIA_FOUNDATION_H
#define VIDEO_CAPTURE_MEDIA_FOUNDATION_H

#include <windows.h>
#include <mfapi.h>
#include <mfplay.h>
#include <mfidl.h>                                                                   /* e.g. MFEnumDeviceSources */
#include <mfreadwrite.h>
#include <mferror.h>                                                                 /* MediaFoundation error codes, MF_E_*  */
#include <shlwapi.h>
#include <string>
#include <vector>
#include <videocapture/Base.h>
#include <videocapture/Types.h>
#include <videocapture/Utils.h>
#include <videocapture/win/MediaFoundation_Types.h>
#include <videocapture/win/MediaFoundation_Utils.h>
#include <videocapture/win/MediaFoundation_Callback.h>

namespace ca {

  class MediaFoundation_Capture : public Base {
  public:
    MediaFoundation_Capture(frame_callback fc, void* user);
    ~MediaFoundation_Capture();

    /* Interface */
    int open(Settings settings);
    int close();
    int start();
    int stop();
    void update();

    /* Capabilities */
    std::vector<Capability> getCapabilities(int device);                               /* Get all the capabilities for the given device number  */
    std::vector<Device> getDevices();                                                  /* We query the udev USB devices. */
    std::vector<Format> getOutputFormats();                                            /* Get the supported output formats. */

  private:

    /* SDK Capabilities */
    int createSourceReader(IMFMediaSource* mediaSource, IMFSourceReaderCallback* callback, IMFSourceReader** sourceReader);   /* Create the IMFSourceReader which is basically an intermediate which allows us to process raw frames ourself. */
    int createVideoDeviceSource(int device, IMFMediaSource** source);                   /* Creates a new IMFMediaSource object that is activated */
    int getCapabilities(IMFMediaSource* source, std::vector<Capability>& result);       /* Get capabilities for the given source */
    int setDeviceFormat(IMFMediaSource* source, DWORD formatIndex);                     /* Set the video capture format */
    int setReaderFormat(IMFSourceReader* reader, Capability& cap);                      /* Set the format for the source reader. The source reader is used to process raw data, see: http://msdn.microsoft.com/en-us/library/windows/desktop/dd940436(v=vs.85).aspx for more information */

  public:
    MediaFoundation_Callback* mf_callback;                                              /* The MediaFoundation_Callback instance that will receive frame data */
    IMFMediaSource* imf_media_source;                                                   /* The IMFMediaSource represents the capture device. */
    IMFSourceReader* imf_source_reader;                                                 /* The IMFSourceReader is an intermediate that must be used to process raw video frames */
    int state;                                                                          /* Used to keep track of open/capture state */
  };
  
  /* Safely release the given obj. */
  template <class T> void safeReleaseMediaFoundation(T **t) {
    if(*t) {
      (*t)->Release();
      *t = NULL;
    }
  }

} // namespace cs
#endif
