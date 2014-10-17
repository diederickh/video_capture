#include <stdio.h>
#include <stdlib.h>
#include <videocapture/decklink/DecklinkDevice.h>

namespace ca {
  
  DecklinkDevice::DecklinkDevice(IDeckLink* device)
    :device(device)
    ,input(NULL)
    ,callback(NULL)
    ,display_mode(bmdModeUnknown)
    ,is_started(false)
    ,fc(NULL)
    ,user(NULL)
  {

    if (NULL == device) {
      printf("Error: the IDeckLink device pointer is NULL. Not supposed to happen. Stopping now.\n");
      exit(EXIT_FAILURE);
    }

    device->AddRef();
  }

  DecklinkDevice::~DecklinkDevice() {

    if (is_started) {
      stop();
    }

    if (NULL != device) {
      device->Release();
    }

    if (NULL != input) {
      input->Release();
    }

    if (NULL != callback) {
      delete callback;
    }

    callback = NULL;
    device = NULL;
    input = NULL;
    is_started = false;
    fc = NULL;
    user = NULL;
  }

  int DecklinkDevice::open(Settings cfg) {

    IDeckLinkDisplayModeIterator* mode_iter = NULL;
    IDeckLinkDisplayMode* mode = NULL;
    HRESULT r = S_OK;
    int i = 0;
    
    if (NULL == device) {
      printf("Error: cannot open DecklinkDevice, because the given/set IDeckLink* is NULL.\n");
      return -1;
    }

    if (NULL != input) {
      printf("Error: the `input` member is not NULL, did you already open the device?\n");
      return -2;
    }

    if (NULL == fc) {
      printf("Error: no frame callback set.\n");
      return -3;
    }
    
    if (bmdModeUnknown != display_mode) {
      printf("Error: the display mode is already set, which means you didn't closed the device before reopening. Please close() first before calling open.\n");
      return -3;
    }

    if (CA_NONE == cfg.capability) {
      printf("Error: capability is not set in DecklinkDevice::open().\n");
      return -4;
    }

    r = device->QueryInterface(IID_IDeckLinkInput, (void**)&input);
    if (S_OK != r) {
      printf("Error: failed to retrieve a IID_IDeckLinkInput interface.\n");
      return -5;
    }

    r = input->GetDisplayModeIterator(&mode_iter);
    if (S_OK != r) {
      printf("Error: cannot get the display mode iterator for the device: %d.\n", cfg.device);
      input->Release();
      input = NULL;
      return -6;
    }

    /* @todo - See Decklink.cpp where we have the same list. */
    const BMDPixelFormat formats[] = { bmdFormat8BitYUV,   /* CA_UYVY422 */
                                       bmdFormat8BitARGB,  /* CA_ARGB32 */
                                       bmdFormat8BitBGRA,  /* CA_BGRA32 */
                                       (BMDPixelFormat)0
    };

    const int formats_map[] = { 
      CA_UYVY422,
      CA_ARGB32,
      CA_RGBA32,
      0
    };

    /* Find the capability */

    while (S_OK == mode_iter->Next(&mode)) {

      BMDDisplayModeSupport mode_support;
      int fmt_dx = 0;
      
      while (formats[fmt_dx] != 0) {

        r = input->DoesSupportVideoMode(mode->GetDisplayMode(), 
                                        formats[fmt_dx], 
                                        bmdVideoInputFlagDefault, 
                                        &mode_support, 
                                        NULL);
        if (S_OK != r) {
          printf("Error: failed to test if video format is supported, fmt_dx: %d\n", fmt_dx);
          break;
        }

        if (bmdDisplayModeNotSupported != mode_support) {
          if (i == cfg.capability) {
            display_mode = mode->GetDisplayMode();
            pixel_format = formats[fmt_dx];
            break;
          }
          i++;
        }
        fmt_dx++;
      }
      mode->Release();

      /* Stop iterating when found. */
      if (bmdModeUnknown != display_mode) {
        break;
      }
    }

    mode_iter->Release();

    if (bmdModeUnknown == display_mode) {
      printf("Error: we couldn't find the capability that was given to DecklinkDevice.\n");
      input->Release();
      input = NULL;
      return -5;
    }

    return 0;
  }

  int DecklinkDevice::close() {

    display_mode = bmdModeUnknown;
    return 0;
  }

  int DecklinkDevice::start() {
    
    HRESULT r = S_OK;

    if (NULL == input) {
      printf("Error: cannot start capturing from the DecklinkDevice because `input` is NULL. Did you open the device?\n");
      return -1;
    }

    if (NULL == fc) {
      printf("Error: no frame callback set, cannot start capturing in DecklinkDevice::start().\n");
      return -3;
    }

    if (bmdModeUnknown == display_mode) {       
      printf("Error: the display mode for the decklink device is still unknown. Not supposed to happen at this point. \n");
      return -2;
    }

    if (NULL == callback) {
      callback = new DecklinkCallback();
      if (NULL == callback) {
        printf("Error: failed to allocate the DecklinkCallback. Out of mem?\n");
        return -3;
      }
    }

    callback->setCallback(fc, user);

    r = input->SetCallback(callback);
    if (S_OK != r) {
      printf("Error: failed to set the callback on the IDeckLinkInput instance.\n");
      delete callback;
      callback = NULL;
      return -4;
    }

    r = input->EnableVideoInput(display_mode, pixel_format, 0);
    if (S_OK != r) {
      printf("Error: failed to set the display mode and enable the video input.\n");
      delete callback;
      callback = NULL;
      return -5;
    }

    r = input->StartStreams();
    if (S_OK != r) {
      printf("Error: failed to start the input streams.\n");
      delete callback;
      callback = NULL;
      return -6;
    }

    is_started = true;

    return 0;
  }

  int DecklinkDevice::stop() {
    
    HRESULT r = S_OK;

    if (false == is_started) {
      printf("Error: cannot stop DecklinkDevice because it's not started yet.\n");
      return -1;
    }

    is_started = false;

    r = input->SetCallback(NULL);
    if (S_OK != r) {
      printf("Error: setting the callback to NULL on the DecklinkDevice failed.");
    }
    
    r = input->StopStreams();
    if (r != S_OK) {
      printf("Error: something went wrong while trying to stop the stream.\n");
      return -2;
    }
    
    return 0;
  }

  void DecklinkDevice::update() {
  }

  
} /* namespace ca */
