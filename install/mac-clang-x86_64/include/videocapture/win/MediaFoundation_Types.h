#ifndef VIDEO_CAPTURE_MEDIA_FOUNDATION_TYPES_H
#define VIDEO_CAPTURE_MEDIA_FOUNDATION_TYPES_H

namespace ca {

  /* -------------------------------------- */
  
  class MediaFoundation_Device {                            /* Wrapper around a Media Foundation device */
  public:
    MediaFoundation_Device();
    ~MediaFoundation_Device();
    void clear();
  };
  
  
} // namespace ca

#endif
