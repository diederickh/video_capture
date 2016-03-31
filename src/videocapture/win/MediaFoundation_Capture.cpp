#include <videocapture/win/MediaFoundation_Capture.h>
#include <iostream>

namespace ca {

  MediaFoundation_Capture::MediaFoundation_Capture(frame_callback fc, void* user) 
    :Base(fc, user)
    ,state(CA_STATE_NONE)
    ,mf_callback(NULL)
    ,imf_media_source(NULL)
    ,imf_source_reader(NULL)
    ,must_shutdown_com(true)
  {
    /* Initialize COM */
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if(FAILED(hr)) {
      
      /* 
         CoInitializeEx must be called at least once and is usually called
         only once freach each thread that uses the COM library. Multiple calls
         to CoInitializeEx by the same thread are allowed as long as they pass
         the same concurrency flag, but subsequent calls returns S_FALSE.

         To close the COM library gracefully on a thread, each successful call to
         CoInitializeEx, including any call that returns S_FALSE, must be balanced
         by a corresponding call to CoUninitialize.         
        
         Source: https://searchcode.com/codesearch/view/76074495/ 
      */
      
      must_shutdown_com = false;
      
      printf("Warning: cannot initialize COM in MediaFoundation_Capture.\n");
    }

    /* Initialize MediaFoundation */
    hr = MFStartup(MF_VERSION);
    if(FAILED(hr)) {
      printf("Error: cannot startup the MediaFoundation_Capture.\n");
      ::exit(EXIT_FAILURE);
    }
  }

  MediaFoundation_Capture::~MediaFoundation_Capture() {

    /* Close and stop */
    if(state & CA_STATE_CAPTUREING) {
      stop();
    }
    
    if(state & CA_STATE_OPENED) {
      close();
    }

    /* Shutdown MediaFoundation */
    HRESULT hr = MFShutdown();
    if(FAILED(hr)) {
      printf("Error: failed to shutdown the MediaFoundation.\n");
    }
    
    /* Shutdown COM */
    if (true == must_shutdown_com) {
      CoUninitialize();
    }

    pixel_buffer.user = NULL;
  }

  int MediaFoundation_Capture::open(Settings settings) {

    if(state & CA_STATE_OPENED) {
      printf("Error: already opened.\n");
      return -1;
    }

    if(imf_media_source) {
      printf("Error: already opened the media source.\n");
      return -2;
    }

    /* Create the MediaSource  */
    if(createVideoDeviceSource(settings.device, &imf_media_source) < 0) {
      printf("Error: cannot create the media device source.\n");
      return -3;
    }

    /* Set the media format, width, height  */
    std::vector<Capability> capabilities;
    if(getCapabilities(imf_media_source, capabilities) < 0) {
      printf("Error: cannot create the capabilities list to open the device.\n");
      safeReleaseMediaFoundation(&imf_media_source);
      return -4;
    }

    if(settings.capability >= capabilities.size()) {
      printf("Error: invalid capability ID, cannot open the device.\n");
      safeReleaseMediaFoundation(&imf_media_source);
      return -5;
    }

    Capability cap = capabilities.at(settings.capability);
    if(cap.pixel_format == CA_NONE) {
      printf("Error: cannot set a pixel format for CA_NONE.\n");
      safeReleaseMediaFoundation(&imf_media_source);
      return -6;
    }

    if(setDeviceFormat(imf_media_source, (DWORD)cap.pixel_format_index) < 0) {
      printf("Error: cannot set the device format.\n");
      safeReleaseMediaFoundation(&imf_media_source);
      return -7;
    }
    
    /* Create the source reader. */
    MediaFoundation_Callback::createInstance(this, &mf_callback);
    if(createSourceReader(imf_media_source, mf_callback, &imf_source_reader) < 0) {
      printf("Error: cannot create the source reader.\n");
      safeReleaseMediaFoundation(&mf_callback);
      safeReleaseMediaFoundation(&imf_media_source);
      return -8;
    }
    
    /* Set the source reader format. */
    if(setReaderFormat(imf_source_reader, cap) < 0) {
      printf("Error: cannot set the reader format.\n");
      safeReleaseMediaFoundation(&mf_callback);
      safeReleaseMediaFoundation(&imf_media_source);
      return -9;
    }

    /* Set the pixel buffer strides, widths and heights based on the selected format. */
    if (0 != pixel_buffer.setup(cap.width, cap.height, cap.pixel_format)) {
      printf("Error: cannot setup the pixel buffer for the current pixel format.\n");
      safeReleaseMediaFoundation(&mf_callback);
      safeReleaseMediaFoundation(&imf_media_source);
      return -10;
    }

    pixel_buffer.user = cb_user;

    state |= CA_STATE_OPENED;

    return 1;
  }

  int MediaFoundation_Capture::close() {
    
    if(!imf_source_reader) {
      printf("Error: cannot close the device because it seems that is hasn't been opend yet. Did you call openDevice?.\n");
      return -1;
    }
    
    if(state & CA_STATE_CAPTUREING) {
      stop();
    }

    safeReleaseMediaFoundation(&imf_source_reader);
    safeReleaseMediaFoundation(&imf_media_source); 
    safeReleaseMediaFoundation(&mf_callback);

    state &= ~CA_STATE_OPENED;

    return 1;
  }

  int MediaFoundation_Capture::start() {

    if(!imf_source_reader) {
      printf("Error: cannot start capture becuase it looks like the device hasn't been opened yet.\n");
      return -1;
    }
    
    if(!(state & CA_STATE_OPENED)) {
      printf("Error: cannot start captureing because you haven't opened the device successfully.\n");
      return -2;
    }

    if(state & CA_STATE_CAPTUREING) {
      printf("Error: cannot start capture because we are already capturing.\n");
      return -3;
    }

    /* Kick off the capture stream. */
    HRESULT hr = imf_source_reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);
    if(FAILED(hr)) {
      if(hr == MF_E_INVALIDREQUEST) {
        printf("ReadSample returned MF_E_INVALIDREQUEST.\n");
      }
      else if(hr == MF_E_INVALIDSTREAMNUMBER) {
        printf("ReadSample returned MF_E_INVALIDSTREAMNUMBER.\n");
      }
      else if(hr == MF_E_NOTACCEPTING) {
        printf("ReadSample returned MF_E_NOTACCEPTING.\n");
      }
      else if(hr == E_INVALIDARG) {
        printf("ReadSample returned E_INVALIDARG.\n");
      }
      else if(hr == E_POINTER) {
        printf("ReadSample returned E_POINTER.\n");
      }
      else {
        printf("ReadSample - unhandled result.\n");
      }
      printf("Error: while trying to ReadSample() on the imf_source_reader. \n");
      std::cout << "Error: " << std::hex << hr << std::endl;
      return -4;
    }

    state |= CA_STATE_CAPTUREING;

    return 1;
  }

  int MediaFoundation_Capture::stop() {

    if(!imf_source_reader) {
      printf("Error: Cannot stop capture because it seems that the device hasn't been opened yet.\n");
      return -1;
    }

    if(!state & CA_STATE_CAPTUREING) {
      printf("Error: Cannot stop capture because we're not capturing yet.\n");
      return -2;
    }

    state &= ~CA_STATE_CAPTUREING;

    return 1;
  }

  void MediaFoundation_Capture::update() {
  }

  std::vector<Device> MediaFoundation_Capture::getDevices() {

    std::vector<Device> result;
    UINT32 count = 0;
    IMFAttributes* config = NULL;
    IMFActivate** devices = NULL;

    HRESULT hr = MFCreateAttributes(&config, 1);
    if(FAILED(hr)) {
      goto done;
    }

    /* Filter capture devices. */
    hr = config->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,  MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if(FAILED(hr)) {
      goto done;
    }
    
    /* Enumerate devices */
    hr = MFEnumDeviceSources(config, &devices, &count);
    if(FAILED(hr)) {
      goto done;
    }

    if(count == 0) {
      goto done;
    }

    for(DWORD i = 0; i < count; ++i) {

      HRESULT hr = S_OK;
      WCHAR* friendly_name = NULL;
      UINT32 friendly_name_len = 0;

      hr = devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,  &friendly_name, &friendly_name_len);
      if(SUCCEEDED(hr)) {
        std::string name = string_cast<std::string>(friendly_name);

        Device dev;
        dev.index = i;
        dev.name = name;
        result.push_back(dev);
      }

      CoTaskMemFree(friendly_name);
    }

  done:
    safeReleaseMediaFoundation(&config);

    for(DWORD i = 0; i < count; ++i) {
      safeReleaseMediaFoundation(&devices[i]);
    }

    CoTaskMemFree(devices);

    return result;
  }

  std::vector<Capability> MediaFoundation_Capture::getCapabilities(int device) {

    std::vector<Capability> result;
    IMFMediaSource* source = NULL;

    if(createVideoDeviceSource(device, &source) > 0) {
      getCapabilities(source, result);
      safeReleaseMediaFoundation(&source);
    }
   
    return result;
  }

  std::vector<Format> MediaFoundation_Capture::getOutputFormats() {
    std::vector<Format> result;
    return result;
  }

  /* PLATFORM SDK SPECIFIC */
  /* -------------------------------------- */

  int MediaFoundation_Capture::setDeviceFormat(IMFMediaSource* source, DWORD formatIndex) {

    IMFPresentationDescriptor* pres_desc = NULL;
    IMFStreamDescriptor* stream_desc = NULL;
    IMFMediaTypeHandler* handler = NULL;
    IMFMediaType* type = NULL;
    int result = 1;

    HRESULT hr = source->CreatePresentationDescriptor(&pres_desc);
    if(FAILED(hr)) {
      printf("source->CreatePresentationDescriptor() failed.\n");
      result = -1;
      goto done;
    }

    BOOL selected;
    hr = pres_desc->GetStreamDescriptorByIndex(0, &selected, &stream_desc);
    if(FAILED(hr)) {
      printf("pres_desc->GetStreamDescriptorByIndex failed.\n");
      result = -2;
      goto done;
    }

    hr = stream_desc->GetMediaTypeHandler(&handler);
    if(FAILED(hr)) {
      printf("stream_desc->GetMediaTypehandler() failed.\n");
      result = -3;
      goto done;
    }

    hr = handler->GetMediaTypeByIndex(formatIndex, &type);
    if(FAILED(hr)) {
      printf("hander->GetMediaTypeByIndex failed.\n");
      result = -4;
      goto done;
    }

    hr = handler->SetCurrentMediaType(type);
    if(FAILED(hr)) {
      printf("handler->SetCurrentMediaType failed.\n");
      result = -5;
      goto done;
    }

  done:
    safeReleaseMediaFoundation(&pres_desc);
    safeReleaseMediaFoundation(&stream_desc);
    safeReleaseMediaFoundation(&handler);
    safeReleaseMediaFoundation(&type);
    return result;
  }

  int MediaFoundation_Capture::createSourceReader(IMFMediaSource* mediaSource,  IMFSourceReaderCallback* callback, IMFSourceReader** sourceReader) {

    if(mediaSource == NULL) {
      printf("Error: Cannot create a source reader because the IMFMediaSource passed into this function is not valid.\n");
      return -1;
    }

    if(callback == NULL) {
      printf("Error: Cannot create a source reader because the calls back passed into this function is not valid.\n");
      return -2;
    }

    HRESULT hr = S_OK;
    IMFAttributes* attrs = NULL;
    int result = 1;
  
    hr = MFCreateAttributes(&attrs, 1);
    if(FAILED(hr)) {
      printf("Error: cannot create attributes for the media source reader.\n");
      result = -3;
      goto done;
    }

    hr = attrs->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, callback);
    if(FAILED(hr)) {
      printf("Error: SetUnknown() failed on the source reader");
      result = -4;
      goto done;
    }

    /* Create a source reader which sets up the pipeline for us so we get access to the pixels */
    hr = MFCreateSourceReaderFromMediaSource(mediaSource, attrs, sourceReader);
    if(FAILED(hr)) {
      printf("Error: while creating a source reader.\n");
      result = -5;
      goto done;
    }

  done:
    safeReleaseMediaFoundation(&attrs);
    return result;
  }
  
  int MediaFoundation_Capture::setReaderFormat(IMFSourceReader* reader, Capability& cap) {

    DWORD media_type_index = 0;
    int result = -1;
    HRESULT hr = S_OK;

    while(SUCCEEDED(hr)) {

      Capability match_cap;
      IMFMediaType* type = NULL;
      hr = imf_source_reader->GetNativeMediaType(0, media_type_index, &type);
    
      if(SUCCEEDED(hr)) {

        /* PIXELFORMAT */
        PROPVARIANT var;
        PropVariantInit(&var);
        {
          hr = type->GetItem(MF_MT_SUBTYPE, &var);
          if(SUCCEEDED(hr)) {
            match_cap.pixel_format = media_foundation_video_format_to_capture_format(*var.puuid); 
          }
        }
        PropVariantClear(&var);

        /* SIZE */
        PropVariantInit(&var);
        {
          hr = type->GetItem(MF_MT_FRAME_SIZE, &var);
          if(SUCCEEDED(hr)) {
            UINT32 high = 0;
            UINT32 low =  0;
            Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &high, &low);
            match_cap.width = high;
            match_cap.height = low;
          }
        }
        PropVariantClear(&var);

        /* FRAME RATE */
        PropVariantInit(&var);
        {
          hr = type->GetItem(MF_MT_FRAME_RATE, &var);
          if (SUCCEEDED(hr)) {
            UINT32 high = 0;
            UINT32 low = 0;
            Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &high, &low);
            match_cap.fps = fps_from_rational(low, high);
          }
        }
        PropVariantClear(&var);
      
        /* When the output media type of the source reader matches our specs, set it! */
        if(match_cap.width == cap.width
           && match_cap.height == cap.height
           && match_cap.pixel_format == cap.pixel_format
           && match_cap.fps == cap.fps)
          {
            hr = imf_source_reader->SetCurrentMediaType(0, NULL, type);
            if(FAILED(hr)) {
              printf("Error: Failed to set the current media type for the given settings.\n");
            }
            else {
              hr = S_OK; 
              result = 1;
            }
          }
        //type->Release();  // tmp moved down and wrapped around safeReleaseMediaFoundation()
      }
      else {
        break;
      }

      safeReleaseMediaFoundation(&type);

      ++media_type_index;
    }

    return result;
  }

  /** 
   * Get capabilities for the given IMFMediaSource which represents 
   * a video capture device.
   *
   * @param IMFMediaSource* source [in]               Pointer to the video capture source.
   * @param std::vector<AVCapability>& caps [out]     This will be filled with capabilites 
   */
  int MediaFoundation_Capture::getCapabilities(IMFMediaSource* source, std::vector<Capability>& caps) {

    IMFPresentationDescriptor* presentation_desc = NULL;
    IMFStreamDescriptor* stream_desc = NULL;
    IMFMediaTypeHandler* media_handler = NULL;
    IMFMediaType* type = NULL;
    int result = 1;

    HRESULT hr = source->CreatePresentationDescriptor(&presentation_desc);
    if(FAILED(hr)) {
      printf("Error: cannot get presentation descriptor.\n");
      result = -1;
      goto done;
    }

    BOOL selected;
    hr = presentation_desc->GetStreamDescriptorByIndex(0, &selected, &stream_desc);
    if(FAILED(hr)) {
      printf("Error: cannot get stream descriptor.\n");
      result = -2;
      goto done;
    }

    hr = stream_desc->GetMediaTypeHandler(&media_handler);
    if(FAILED(hr)) {
      printf("Error: cannot get media type handler.\n");
      result = -3;
      goto done;
    }

    DWORD types_count = 0;
    hr = media_handler->GetMediaTypeCount(&types_count);
    if(FAILED(hr)) {
      printf("Error: cannot get media type count.\n");
      result = -4;
      goto done;
    }

#if 0
    // The list of supported types is not garantueed to return everything :) 
    // this was a test to check if some types that are supported by my test-webcam
    // were supported when I check them manually. (they didn't).
    // See the Remark here for more info: http://msdn.microsoft.com/en-us/library/windows/desktop/bb970473(v=vs.85).aspx
    IMFMediaType* test_type = NULL;
    MFCreateMediaType(&test_type);
    if(test_type) {
      GUID types[] = { MFVideoFormat_UYVY, 
                       MFVideoFormat_I420,
                       MFVideoFormat_IYUV, 
                       MFVideoFormat_NV12, 
                       MFVideoFormat_YUY2, 
                       MFVideoFormat_Y42T,
                       MFVideoFormat_RGB24 } ;

      test_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
      for(int i = 0; i < 7; ++i) {
        test_type->SetGUID(MF_MT_SUBTYPE, types[i]);
        hr = media_handler->IsMediaTypeSupported(test_type, NULL);
        if(hr != S_OK) {
          printf("> Not supported: %d\n");
        }
        else {
          printf("> Yes, supported: %d\n", i);
        }
      }
    }
    safeReleaseMediaFoundation(&test_type);
#endif

    // Loop over all the types
    PROPVARIANT var;
    for(DWORD i = 0; i < types_count; ++i) {

      Capability cap;

      hr = media_handler->GetMediaTypeByIndex(i, &type);

      if(FAILED(hr)) {
        printf("Error: cannot get media type by index.\n");
        result = -5;
        goto done;
      }
    
      UINT32 attr_count = 0;
      hr = type->GetCount(&attr_count);
      if(FAILED(hr)) {
        printf("Error: cannot type param count.\n");
        result = -6;
        goto done;
      }

      if(attr_count > 0) {
        for(UINT32 j = 0; j < attr_count; ++j) {

          GUID guid = { 0 };
          PropVariantInit(&var);

          hr = type->GetItemByIndex(j, &guid, &var);
          if(FAILED(hr)) {
            printf("Error: cannot get item by index.\n");
            result = -7;
            goto done;
          }

          if(guid == MF_MT_SUBTYPE && var.vt == VT_CLSID) {
            cap.pixel_format = media_foundation_video_format_to_capture_format(*var.puuid);
            cap.pixel_format_index = i;
          }
          else if(guid == MF_MT_FRAME_SIZE) {
            UINT32 high = 0;
            UINT32 low =  0;
            Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &high, &low);
            cap.width = (int)high;
            cap.height = (int)low;
          }
          else if(guid == MF_MT_FRAME_RATE) {
              // @todo - use MF_MT_FRAME_RATE_RANGE_MIN and MF_MT_FRAME_RATE_RANGE_MAX to extract more FPS variants supported by the device - important: modify also setReaderFormat() 
			  // @todo - not all FPS are added to the capability list. 
              UINT32 high = 0;
              UINT32 low =  0;
              Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &high, &low);
              cap.fps = fps_from_rational(low, high);
              cap.fps_index = i;
            }

          PropVariantClear(&var);
        }

        cap.capability_index = i;
        caps.push_back(cap);
      }

      safeReleaseMediaFoundation(&type);
    }

  done: 
    safeReleaseMediaFoundation(&presentation_desc);
    safeReleaseMediaFoundation(&stream_desc);
    safeReleaseMediaFoundation(&media_handler);
    safeReleaseMediaFoundation(&type);
    PropVariantClear(&var);
    return result;
  }

  /**
   * Create and active the given `device`. 
   *
   * @param int device [in]            The device index for which you want to get an
   *                                   activated IMFMediaSource object. This function 
   *                                   allocates this object and increases the reference
   *                                   count. When you're ready with this object, make sure
   *                                   to call `safeReleaseMediaFoundation(&source)`
   *
   * @param IMFMediaSource** [out]     We allocate and activate the device for the 
   *                                   given `device` parameter. When ready, call
   *                                   `safeReleaseMediaFoundation(&source)` to free memory.
   */
  int MediaFoundation_Capture::createVideoDeviceSource(int device, IMFMediaSource** source) {

    int result = 1;
    IMFAttributes* config = NULL;
    IMFActivate** devices = NULL;
    UINT32 count = 0;  

    HRESULT hr = MFCreateAttributes(&config, 1);
    if(FAILED(hr)) {
      result = -1;
      goto done;
    }

    /* Filter on capture devices */
    hr = config->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if(FAILED(hr)) {
      printf("Error: cannot set the GUID on the IMFAttributes*.\n");
      result = -2;
      goto done;
    }

    /* Enumerate devices. */
    hr = MFEnumDeviceSources(config, &devices, &count);
    if(FAILED(hr)) {
      printf("Error: cannot get EnumDeviceSources.\n");
      result = -3;
      goto done;
    }
    if(count == 0 || device > count) {
      result = -4;
      goto done;
    }

    /* Make sure the given source is free/released. */
    safeReleaseMediaFoundation(source);

    /* Activate the capture device. */
    hr = devices[device]->ActivateObject(IID_PPV_ARGS(source));
    if(FAILED(hr)) {
      printf("Error: cannot activate the object.");
      result = -5;
      goto done;
    }

    result = true;

  done:

    safeReleaseMediaFoundation(&config);
    for(DWORD i = 0; i < count; ++i) {
      safeReleaseMediaFoundation(&devices[i]);
    }
    CoTaskMemFree(devices);

    return result;
  }
} /* namespace ca */
