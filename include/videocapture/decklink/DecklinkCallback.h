#ifndef VIDEO_CAPTURE_DECKLINK_CALLBACK_H
#define VIDEO_CAPTURE_DECKLINK_CALLBACK_H

#include <videocapture/Types.h>
#include <decklink/DecklinkAPI.h>

namespace ca {

  class DecklinkCallback : public IDeckLinkInputCallback {
  public:
    DecklinkCallback();
    ~DecklinkCallback();
    void setCallback(frame_callback cb, void* user);

    /* IUnknown */
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) { return E_NOINTERFACE; } 
    virtual ULONG STDMETHODCALLTYPE AddRef(void) { return 1; }
    virtual ULONG STDMETHODCALLTYPE Release(void) { return 1; } 

    /* IDecklinkInputCallback */
    virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
    virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);

    frame_callback fc;           /* Will be called when we received a video frame (may happen from a different thread. */ 
    void* user;                  /* Is passed into the frame callback. */
  };

  /* ------------------------------------------------------------------------- */

  inline void DecklinkCallback::setCallback(frame_callback cb, void* usr) {

    if(NULL == cb) {
      printf("Error: trying to set a callback but a NULL callback is given.\n");
      return;
    }
    
    fc = cb;
    user = usr;
  }

} /* namespace ca */


#endif
