#include <videocapture/decklink/DecklinkDevice.h>
#include <videocapture/decklink/Decklink.h>

#if defined(_WIN32)
#  include <comutil.h>
#  include <tchar.h>
#endif

namespace ca {

#if defined(_WIN32)
  bool Decklink::is_com_initialized = false;
#endif

  /* -------------------------------------------------------------------- */

  Decklink::Decklink() {

#if defined(_WIN32)
    if (false == is_com_initialized) {
      CoInitialize(NULL);
    }
#endif

  }

  Decklink::~Decklink() {
  }

  int Decklink::listDevices() {

    std::vector<Device> devs = getDevices();

    return 0;
  }

  std::vector<Device> Decklink::getDevices() {
    
    HRESULT r = S_OK;
    IDeckLinkIterator* iter = NULL;
    std::vector<Device> devs;

#if defined(_WIN32)

    iter = NULL;
    r = CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)&iter);
    if (S_OK != r) {
      printf("Error: failed to create a DeckLink device iterator.");
      return devs;
    }

#else 

    iter = CreateDeckLinkIteratorInstance();
    if (NULL == iter) {
      printf("Error: failed to get a decklink iterator. Do you have decklink devices installed?\n");
      return devs;
    }

#endif

    int i = 0;
    IDeckLink* dl = NULL;

#if defined(_WIN32)

    BSTR device_name = NULL;
    while (S_OK == iter->Next(&dl)) {
      printf("Found a decklink device!\n");

      r = dl->GetModelName(&device_name);
      if (S_OK != r) {
        printf("Error: cannot retrieve the device name.");
      }

      _bstr_t devname(device_name, false);

      Device dev;
      dev.index = i;
      dev.name = (char*)devname;
      devs.push_back(dev);
      
      dl->Release();
      dl = NULL;
      ++i;
    }

#else
#  error "Not implement on other OS then Win."
#endif
    
    iter->Release();
    iter = NULL;

    return devs;
  }

  std::vector<Capability> Decklink::getCapabilities(int index) {

    std::vector<Capability> caps;
    HRESULT r = S_OK;
    IDeckLink* device = NULL;
    IDeckLinkInput* input = NULL;
    IDeckLinkDisplayModeIterator* mode_iter = NULL;
    IDeckLinkDisplayMode* mode = NULL;
    int i = 0;

    device = getDevice(index);
    if (NULL == device) {
      printf("Error: cannot find the given device: %d\n", index);
      return caps;
    }
    
    r = device->QueryInterface(IID_IDeckLinkInput, (void**)&input);
    if (S_OK != r) {
      printf("Error: cannot get the input interface for device: %d.\n", index);
      device->Release();
      return caps;
    }

    r = input->GetDisplayModeIterator(&mode_iter);
    if (S_OK != r) {
      printf("Error: cannot get the display mode iterator for the device: %d.\n", index);
      input->Release();
      device->Release();
    }
    
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

    while (S_OK == mode_iter->Next(&mode)) {

      BSTR mode_bstr = NULL;
      BMDTimeValue frame_num;
      BMDTimeValue frame_den;
      BMDDisplayModeSupport mode_support;
      int fmt_dx = 0;
      
      r = mode->GetName(&mode_bstr);
      if (S_OK != r) {
        printf("Error: failed to get display mode (capability) name for device: %d.\n", index);
        mode->Release();
        continue;
      }

      r = mode->GetFrameRate(&frame_num, &frame_den);
      if (S_OK != r) {
        printf("Error: failed to get the frame rate (capability) for device: %d.\n", index);
        mode->Release();
        continue;
      }

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
          Capability cap;
          cap.width = (int) mode->GetWidth();
          cap.height = (int) mode->GetHeight();
          cap.fps = fps_from_rational(frame_num, frame_den);
          cap.capability_index = i;
          caps.push_back(cap);
          i++;
        }

        fmt_dx++;
      }

      mode->Release();
    }

#if 0
    /* Set by the user */
    int width;                                                                      /* Width for this capability. */
    int height;                                                                     /* Height for this capability. */
    int pixel_format;                                                               /* The pixel format for this capability. */
    int fps;                                                                        /* The FPS, see CA_FPS_* above. */
    
    /* Set by the capturer implementation */
    int capability_index;                                                           /* Used by the implementation. Is the ID of this specific capability */
    int fps_index;                                                                  /* Used by the implementation, can be an index to an FPS array that is provided by the implementation */              
    int pixel_format_index;                                                         /* Used by the implementation, represents an index to the pixel format for te implementation */
    void* user;                                                                     /* Can be set by the implementation to anything which is suitable */

#endif   

    device->Release();

    return caps;
  }

  /*
    Get a IDeckLink pointer. The user needs to call ->Release()
     on the returned pointer when it's not used anymore.
  */
  IDeckLink* Decklink::getDevice(int index) {
    
    HRESULT r = S_OK;
    IDeckLinkIterator* iter = NULL;
    std::vector<Device> devs;

#if defined(_WIN32)

    iter = NULL;
    r = CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)&iter);
    if (S_OK != r) {
      printf("Error: failed to create a DeckLink device iterator.");
      return NULL;
    }

#else 

    iter = CreateDeckLinkIteratorInstance();
    if (NULL == iter) {
      printf("Error: failed to get a decklink iterator. Do you have decklink devices installed?\n");
      return devs;
    }

#endif

    int i = 0;
    IDeckLink* dl = NULL;
    IDeckLink* result = NULL;

    while (S_OK == iter->Next(&dl)) {

      if (i == index) {
        result = dl;
        result->AddRef();
      }
      
      dl->Release();
      dl = NULL;
      ++i;
    }

    
    iter->Release();
    iter = NULL;

    return result;
  }


} /* namespace ca */
