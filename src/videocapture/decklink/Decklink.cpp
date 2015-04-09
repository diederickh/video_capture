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

  Decklink::Decklink(frame_callback fc, void* user) 
    :Base(fc, user)
    ,decklink_device(NULL)
  {

#if defined(_WIN32)
    if (false == is_com_initialized) {
      /* 
         See notes in MediaCapture_Foundation about CoInitialize ... 
         @todo we should use the same init.
      */
      CoInitialize(NULL);
    }
#endif

  }

  Decklink::~Decklink() {

    if (decklink_device) {
      delete decklink_device;
    }

    decklink_device = NULL;
  }

  int Decklink::open(Settings settings) {

    if (NULL != decklink_device) {
      printf("Error: you already opened a decklink device. First close it before opening again.\n");
      return -1;
    }

    /* @todo for now we only support capabilities, so settings.capability must be set. */
    std::vector<Capability> caps = getCapabilities(settings.device);
    if (settings.capability >= caps.size()) {
      printf("Error: Invalid capabilty: %d in Decklink::open(), caps.size(): %lu.\n", settings.capability, caps.size());
    }

    IDeckLink* device = getDevice(settings.device);
    if (NULL == device) {
      printf("Error: cannot find the device with id: %d.\n", settings.device);
      return -2;
    }

    decklink_device = new DecklinkDevice(device);
    if (NULL == decklink_device) {
      printf("Error: failed to allocate the DecklinkDevice wrapper. Out of mem?\n");
      device->Release();
      device = NULL; 
      return -3;
    }

    decklink_device->setCallback(cb_frame, cb_user);

    if (decklink_device->open(settings) < 0) {
      printf("Error: failed to open the decklink device implementation.\n");
      delete decklink_device;
      decklink_device = NULL;
      device->Release();
      device = NULL;
      return -4;
    }

    device->Release();
    device = NULL;

    return 0;
  }

  int Decklink::close() {
    
    int r = 0;

    if (NULL == decklink_device) {
      printf("Error: asked to close the Decklink but it looks like you didn't open it. \n");
      return -1;
    }

    if (decklink_device->close() < 0) {
      printf("Error: closing the DecklinkDevice returned an error.\n");
      r = -2;
    }

    delete decklink_device;
    decklink_device = NULL;

    return r;
  }

  int Decklink::start() {

    if (NULL == decklink_device) {
      printf("Error: cannot start the decklink device because it's not opened yet. Call open() first.\n");
      return -1;
    }

    return decklink_device->start();
  }

  int Decklink::stop() {

    if (NULL == decklink_device) {
      printf("Error: cannot stop the decklink device because it's not opened yet. Call open() first.\n");
      return -1;
    }

    return decklink_device->stop();
  }

  void Decklink::update() {
  }

#if 0
  int Decklink::listDevices() {

    std::vector<Device> devs = getDevices();

    return 0;
  }
#endif

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

  std::vector<Format> Decklink::getOutputFormats() {
    std::vector<Format> fmts;
    return fmts;
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

    /* @todo see DecklinkDevice which has a copy of this list. */
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
          
          _bstr_t bs(mode_bstr, false);

          Capability cap;
          cap.width = (int) mode->GetWidth();
          cap.height = (int) mode->GetHeight();
          cap.fps = fps_from_rational(frame_num, frame_den);
          cap.pixel_format = formats_map[fmt_dx];
          cap.capability_index = i;
          cap.fps_index = 0;
          cap.pixel_format_index = fmt_dx;
          cap.description = bs;
          cap.user = NULL;
          caps.push_back(cap);
          i++;
        }

        fmt_dx++;
      }

      mode->Release();
    }

    device->Release();
    input->Release();
    mode_iter->Release();

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
