/*

  Asynchronous Source Reader
  ---------------------------
  See http://msdn.microsoft.com/en-us/library/windows/desktop/gg583871(v=vs.85).aspx for more information.

 */
#ifndef VIDEO_CAPTURE_MEDIA_FOUNDATION_CALLBACK_H
#define VIDEO_CAPTURE_MEDIA_FOUNDATION_CALLBACK_H

#include <windows.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <shlwapi.h>

namespace ca { 

  class MediaFoundation_Capture;
                                         
  class MediaFoundation_Callback : public IMFSourceReaderCallback {
  public:
    static bool createInstance(MediaFoundation_Capture* cap, MediaFoundation_Callback** cb);

    STDMETHODIMP QueryInterface(REFIID iid, void** v);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
  
    STDMETHODIMP OnReadSample(HRESULT hr, DWORD streamIndex, DWORD streamFlags, LONGLONG timestamp, IMFSample* sample);
    STDMETHODIMP OnEvent(DWORD streamIndex, IMFMediaEvent* event);
    STDMETHODIMP OnFlush(DWORD streamIndex);

    HRESULT Wait(DWORD* streamFlags, LONGLONG* timestamp, IMFSample* sample) { return S_OK;  }
    HRESULT Cancel() { return S_OK; } 

  private:
    MediaFoundation_Callback(MediaFoundation_Capture* cap);
    virtual ~MediaFoundation_Callback();

  private:
    MediaFoundation_Capture* cap;
    long ref_count;
    CRITICAL_SECTION crit_sec;
  };

} // namespace ca

#endif
