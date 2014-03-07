#ifndef VIDEO_CAPTURE_V4L2_TYPES_H        
#define VIDEO_CAPTURE_V4L2_TYPES_H

#include <videocapture/Types.h>

namespace ca {

  /* -------------------------------------- */

  class V4L2_Buffer {                              /* A V4L2_Buffer is used to store the frames of the camera */
  public:
    V4L2_Buffer();
    ~V4L2_Buffer();
    void clear();                                  /* Sets the buffer to NULL and size to 0. IMPORTANT: we do not free any allocated memory; the user of this buffer should do that! */

  public:
    void* start;
    size_t length;
  };

  /* -------------------------------------- */

  class V4L2_Device {                              /* Represents a V4L2 device */
  public:
    V4L2_Device();
    ~V4L2_Device();
    void clear();
    std::string toString();

  public:
    std::string path;
    std::string id_vendor;
    std::string id_product;
    std::string driver;
    std::string card;
    std::string bus_info;
    int version_major;
    int version_minor;
    int version_micro;
  };

} // namespace ca

#endif
