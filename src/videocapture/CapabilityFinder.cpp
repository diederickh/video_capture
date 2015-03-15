#include <videocapture/CapabilityFinder.h>

namespace ca {

  /* ------------------------------------------------------------------------- */

  static bool sort_capability(const Capability& a, const Capability& b);       /* Used when filtering capabilities. This will sort the capabilites on the filter_score. */
  
  /* ------------------------------------------------------------------------- */

  CapabilityFinder::CapabilityFinder(Capture& cap)
    :cap(cap)
  {
  }
  
  CapabilityFinder::~CapabilityFinder() {
    filters.clear();
  }

  int CapabilityFinder::addFilter(int attribute, double value, int priority) {

    if (CA_WIDTH != attribute
        && CA_HEIGHT != attribute
        && CA_RATIO != attribute
        && CA_PIXEL_FORMAT != attribute)
      {
        printf("Error: invalid attribute.\n");
        return -1;
      }
    
    filters.push_back(CapabilityFilter(attribute, value, priority));

    return 0;
  }

  int CapabilityFinder::findSettingsForFormat(int device, int format, Settings& result) {

    result.device = device;
    
    if (0 == filters.size()) {
      printf("Error: cannot get the best capability because you haven't added any filters.\n");
      return -1;
    }
    
    std::vector<Capability> capabilities = filterCapabilities(device);
    if (0 == capabilities.size()) {
      printf("Error: cannot find any capability which conforms the set filters.\n");
      return -2;
    }

    Capability best_capability = capabilities[0];
    if (0 > best_capability.index) {
      printf("Error: the index value < 0; not supposed to happen.\n");
      return -3;
    }

    /* Check if the found capability uses the requested format, and if not check if we can convert it*/
    if (best_capability.pixel_format != format) {
      if (0 == cap.hasOutputFormat(format)) {
        result.format = format;
      }
      else {
        return -4;
      }
    }

    result.capability = best_capability.index;

    return 0;
  }

  std::vector<Capability> CapabilityFinder::filterCapabilities(int device) {

    float ratio = 0.0;
    std::vector<Capability> result;
    std::vector<Capability> capabilities = cap.getCapabilities(device);

    if (0 == filters.size()) {
      return result;
    }
    
    if (0 == capabilities.size()) {
      return result;
    }

    for (size_t i = 0; i < filters.size(); ++i) {
      
      CapabilityFilter& filter = filters[i];
      
      for (size_t j = 0; j < capabilities.size(); ++j) {
        
        Capability& capability = capabilities[j];
        capability.index = j;

        switch(filter.attribute) {
          
          case CA_WIDTH: {
            if ((int)filter.value == capability.width) {
              capability.filter_score += filter.priority;
            }
            break;
          }
            
          case CA_HEIGHT: {
            if ((int)filter.value == capability.height) {
              capability.filter_score += filter.priority;
            }
            break;
          }
            
          case CA_RATIO: {
            
            if (0 == capability.width || 0 == capability.height) {
              printf("Error: the capability is missing a value for it's width and/or height: %d x %d.\n", capability.width, capability.height);
              continue;
            }
        
            ratio = double(capability.width) / capability.height;
            if (filter.value == ratio) {
              capability.filter_score += filter.priority;
            }
            
            break;
          }
            
          case CA_PIXEL_FORMAT: {
            if ((int)filter.value == capability.pixel_format) {
              capability.filter_score += filter.priority;
            }
            break;
          }
          default: {
            printf("Unhandled capability filter attribute: %d\n", filter.attribute);
            break;
          }
        }
      }
    }
    
    std::sort(capabilities.begin(), capabilities.end(), sort_capability);

    for (size_t i = 0; i < capabilities.size(); ++i) {
      
      Capability& capability = capabilities[i];
      if (capability.filter_score == 0) {
        continue;
      }

      result.push_back(capability);
    }

    return result;
  }
      
  /* ------------------------------------------------------------------------- */

  static bool sort_capability(const Capability& a, const Capability& b) {
    
    if (a.filter_score != b.filter_score) {
      return a.filter_score > b.filter_score;
    }

    return a.fps > b.fps;
  }

  
} /* namespace ca */
 
