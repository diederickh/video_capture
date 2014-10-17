#if defined(__linux)
#  define  __STDC_LIMIT_MACROS 
#endif
#include <stdint.h>
#include <stdio.h>
#include <videocapture/decklink/DecklinkCallback.h>

namespace ca {

  DecklinkCallback::DecklinkCallback() 
    :fc(NULL)
    ,user(NULL)
  {
  }

  DecklinkCallback::~DecklinkCallback() {
    fc = NULL;
    user = NULL;
  }

  HRESULT STDMETHODCALLTYPE DecklinkCallback::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents ev, 
                                                                      IDeckLinkDisplayMode* mode, 
                                                                      BMDDetectedVideoInputFormatFlags falgs)
  {
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE DecklinkCallback::VideoInputFrameArrived(IDeckLinkVideoInputFrame* vframe,
                                                                     IDeckLinkAudioInputPacket* aframe)
  {
    HRESULT r = S_OK;
    char* pix = NULL;
    long nbytes = 0;

#if !defined(NDEBUG)

    if (NULL == fc) {                           
      printf("Error: DecklinkCallback::VideoInputFrameArrived(): no callback set. Stopping.\n");
      ::exit(EXIT_FAILURE);
    }

    if (NULL == vframe) {
      printf("Error: DecklinkCallback::VideoInputFrameArrived(): vframe is NULL. Stopping.\n");
      ::exit(EXIT_FAILURE);
    }

#endif    

    nbytes = vframe->GetRowBytes() * vframe->GetHeight();

#if !defined(NDEBUG)
    if (INT32_MAX < nbytes) {
      printf("Error: then number of bytes in the current video frame is to large to hold in a int. This is not supposed to happen.\n");
      ::exit(EXIT_FAILURE);
    }
#endif

    r = vframe->GetBytes((void**)&pix);

    if (S_OK != r) {
      printf("Error: failed to get bytes in the decklink callback.\n");
    }
    else {
       fc(pix, (int)nbytes, user);
    }

    return S_OK;
  }

} /* namespace ca */
