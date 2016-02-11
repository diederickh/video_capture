#include <videocapture/win/MediaFoundation_Capture.h>
#include <videocapture/win/MediaFoundation_Callback.h>
#include <assert.h>
#include <mfidl.h>
#include <shlwapi.h>

namespace ca { 

  bool MediaFoundation_Callback::createInstance(MediaFoundation_Capture* cap, MediaFoundation_Callback** cb) {

    if(cb == NULL) {
      printf("Error: the given MediaFoundation_Capture is invalid; cant create an instance.\n");
      return false;
    }

    MediaFoundation_Callback* media_cb = new MediaFoundation_Callback(cap);
    if(!media_cb) {
      printf("Error: cannot allocate a MediaFoundation_Callback object - out of memory\n");
      return false;
    }
  
    *cb = media_cb;
    (*cb)->AddRef();

    safeReleaseMediaFoundation(&media_cb); 
    return true;
  }

  MediaFoundation_Callback::MediaFoundation_Callback(MediaFoundation_Capture* cap) 
    :ref_count(1)
    ,cap(cap)
  {
    InitializeCriticalSection(&crit_sec);
  }

  MediaFoundation_Callback::~MediaFoundation_Callback() {
  }
  
  HRESULT MediaFoundation_Callback::QueryInterface(REFIID iid, void** v) {
    static const QITAB qit[] = {
      QITABENT(MediaFoundation_Callback, IMFSourceReaderCallback), { 0 },
    };
    return QISearch(this, qit, iid, v);
  }

  ULONG MediaFoundation_Callback::AddRef() {
    return InterlockedIncrement(&ref_count);
  }

  ULONG MediaFoundation_Callback::Release() {
    ULONG ucount = InterlockedDecrement(&ref_count);
    if(ucount == 0) {
      delete this;
    }
    return ucount;
  }

  HRESULT MediaFoundation_Callback::OnReadSample(HRESULT hr, DWORD streamIndex, DWORD streamFlags, LONGLONG timestamp, IMFSample* sample) {
    assert(cap);
    assert(cap->imf_source_reader);
    assert(cap->cb_frame);

    EnterCriticalSection(&crit_sec);

    if(SUCCEEDED(hr) && sample) {

      IMFMediaBuffer* buffer;
      HRESULT hr = S_OK;
      DWORD count = 0;
      sample->GetBufferCount(&count);

      for(DWORD i = 0; i < count; ++i) {

        hr = sample->GetBufferByIndex(i, &buffer);

        
        if(SUCCEEDED(hr)) {
       
          DWORD length = 0;
          DWORD max_length = 0;
          BYTE* data = NULL;
          buffer->Lock(&data, &max_length, &length);
          
          cap->pixel_buffer.nbytes = (size_t)length;
          cap->pixel_buffer.plane[0] = data;
          if (cap->pixel_buffer.offset[1] > 0) {
            cap->pixel_buffer.plane[1] = data + cap->pixel_buffer.offset[1];
          }
          if (cap->pixel_buffer.offset[2] > 0) {
            cap->pixel_buffer.plane[2] = data + cap->pixel_buffer.offset[2];
          }
          cap->cb_frame(cap->pixel_buffer);

          buffer->Unlock();
          buffer->Release();
        }
      }
    }

    if(SUCCEEDED(hr)) {
      if(cap->imf_source_reader && cap->state & CA_STATE_CAPTUREING) {
        hr = cap->imf_source_reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);
        if(FAILED(hr)) {
          printf("Error: while trying to read the next sample.\n");
        }
      }
    }

    LeaveCriticalSection(&crit_sec);
    return S_OK;
  }

  HRESULT MediaFoundation_Callback::OnEvent(DWORD, IMFMediaEvent* event) {
    return S_OK;
  }

  HRESULT MediaFoundation_Callback::OnFlush(DWORD) {
    return S_OK;
  }

} // namespace ca
