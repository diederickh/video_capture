#include <sstream>
#include <videocapture/linux/V4L2_Types.h>

namespace ca {

  /* V4L2_Buffer */
  /* -------------------------------------- */
  V4L2_Buffer::V4L2_Buffer() {
    clear();
  }

  V4L2_Buffer::~V4L2_Buffer() {
    clear();
  }

  void V4L2_Buffer::clear() {
    // Note that the user needs to free "start" 
    start = NULL;
    length = 0;
  }

  /* V4L2_Device */
  /* -------------------------------------- */

  V4L2_Device::V4L2_Device() {
    clear();
  }

  V4L2_Device::~V4L2_Device() {
    clear();
  }

  void V4L2_Device::clear() {

    path.clear();
    id_vendor.clear();
    id_product.clear();
    driver.clear();
    card.clear();
    bus_info.clear();
    version_major = CA_NONE;
    version_minor = CA_NONE;
    version_micro = CA_NONE;
  }

  std::string V4L2_Device::toString() {

    std::stringstream ss;

    ss << "idVendor: "   << id_vendor   << ", " 
       << "idProduct: "  << id_product  << ", "
       << "driver: "     << driver      << ", " 
       << "card: "       << card        << ", "
       << "bufinfo: "    << bus_info    << ", "
       << "version: "    << version_major << "." << version_minor << "." << version_micro << ", "
       << "path: "       << path;
    

    std::string result = ss.str();
    return result;
  }
};
