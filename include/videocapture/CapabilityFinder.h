/* -*-c++-*- */
/*

  Capability Filtering
  --------------------

  Often you don't know what webcam/capture device will be used though you do want a 
  specific pixel format, width, height or resolution. By using the `CapabilityFinder`
  class you can specify what attributes you find most important and filter on that. 
  Possible filters are:
  
     - CA_WIDTH
     - CA_HEIGHT
     - CA_RATIO
     - CA_PIXEL_FORMAT

  You add these filters to the `CapabilityFinder` instance and then use e.g.
  `findSettingsForFormat()` which returns 0 on success and it will set the 
  given `Settings` parameter correctly. This also means that it will set
  the `Settings.format` parameter which is used to convert from a capture-input
  format to a output format the the specific capture SDK supports if necesary. On 
  Mac for  example, when you use a Logitech C920 webcam, it can only capture at 1280x720
  using the CA_JPEG_OPENDML format that the camera seems to provide (atm). Though 
  often it's handy to use a YUV format in your application; Mac supports automatic
  conversion from CA_JPEG_OPENDML to CA_YUYV422. In this case we set the format
  member of the given `Settings` parameter in `findSettingsForFormat()`.

  Quick Tip!
  ----------
  
  Define your filters in groups. For example if the pixel format is most important
  define it's values in a certain range which doesn't overlap with other filters. 
  For examnple, choose 80-100 as the priority value for the pixel format values 
  60-79 for the width, 40-59 for the height, etc.. 

  Example
  -------
  See `src/test_capability_filter.cpp`.

 */
#ifndef VIDEO_CAPTURE_CAPABILITY_FINDER_H
#define VIDEO_CAPTURE_CAPABILITY_FINDER_H

#include <algorithm>
#include <videocapture/Capture.h>
#include <videocapture/Types.h>

namespace ca {
  
class CapabilityFinder {
 public:
  CapabilityFinder(Capture& capture);                                     /* Initialize and pass the Capture reference that we use to retrieve capabilities from a device. */
  ~CapabilityFinder();                                                    /* Cleans up the added filters. */
  int addFilter(int attribute, double value, int priority);               /* Add a filter, see the info at the top of this document for more info. */
  int findSettingsForFormat(int device, int format, Settings& result);    /* Will return the best matching Settings (if found) based on the added filters. */ 
  std::vector<Capability> filterCapabilities(int device);                 /* Returns an vector of capabilities that matches any of the added filters. */
  
 public:
  Capture& cap;                                                           /* Reference to the capture instance. */
  std::vector<CapabilityFilter> filters;                                  /* The added filters. */
};
 
} /* namespace ca */

#endif
