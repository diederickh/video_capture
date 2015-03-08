/*
  
  Capability Filtering
  --------------------

  Often you don't know what webcam device will be used though you do want a 
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
  format to a output format the the specific capture SDK supports. On Mac for 
  example, when you use a Logitech C920 webcam, it can only capture at 1280x720
  using the CA_JPEG_OPENDML format that the camera seems to provide. Though 
  often it's handy to use a YUV format in your application; Mac supports automatic
  conversion from CA_JPEG_OPENDML to CA_YUYV422. In this case we set the format
  member of the given `Settings` parameter in `findSettingsForFormat()`.

  Quick Tip!
  ----------
  
  Define your filters in groups. For example if the pixel format is most important
  define it's values in a certain range which doesn't overlap with other filters. 
  For examnple, choose 80-100 as the priority value for the pixel format values 
  60-79 for the width, 40-59 for the height, etc.. 

 */
#include <stdio.h>
#include <stdlib.h>
#include <videocapture/Capture.h>
#include <videocapture/Utils.h>
#include <videocapture/CapabilityFinder.h>

using namespace ca;

static void fcallback(PixelBuffer& buffer); 

int main() {

  printf("\nCapability Filtering Example.\n\n");
  
  int device = 1;
  Capture cap(fcallback, NULL);

  CapabilityFinder finder(cap);
  finder.addFilter(CA_PIXEL_FORMAT, CA_YUYV422, 100);
  finder.addFilter(CA_PIXEL_FORMAT, CA_UYVY422, 100);
  finder.addFilter(CA_PIXEL_FORMAT, CA_JPEG_OPENDML, 100);
  finder.addFilter(CA_WIDTH, 1280, 95);
  finder.addFilter(CA_HEIGHT, 720, 95);
  finder.addFilter(CA_WIDTH, 800, 90);
  finder.addFilter(CA_HEIGHT, 600, 90);
  finder.addFilter(CA_WIDTH, 640, 80);
  finder.addFilter(CA_HEIGHT, 480, 80);

  Settings settings_format;
  if (0 == finder.findSettingsForFormat(device, CA_UYVY422, settings_format)) {
    printf("Best matching for format, capability: %d, format: %s\n", settings_format.capability, format_to_string(settings_format.format).c_str());
  }
  
  return 0;
}


static void fcallback(PixelBuffer& buffer) {
  printf("Received a frame with format: %s\n", format_to_string(buffer.pixel_format).c_str());
}
