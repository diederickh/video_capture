#ifndef VIDEO_CAPTURE_MEDIA_FOUNDATION_UTILS_H
#define VIDEO_CAPTURE_MEDIA_FOUNDATION_UTILS_H

#include <vector>
#include <string>
#include <cstring>
#include <cwchar>
#include <cassert>
#include <mfapi.h>
#include <mfplay.h>
#include <mfidl.h>                                                                   /* e.g. MFEnumDeviceSources */
#include <mfreadwrite.h>
#include <shlwapi.h>
#include <string>
#include <videocapture/Types.h>

namespace ca {

  std::string media_foundation_video_format_to_string(const GUID& guid);           /* Convert a MF format to a string */
  int media_foundation_video_format_to_capture_format(GUID guid);                  /* Convert a MF format to a capture format */

  // Convert a WCHAR to a std::string
  template<class T>
    T string_cast( const wchar_t* src, unsigned int codePage = CP_ACP) {

    assert(src != 0);
    size_t source_length = std::wcslen(src);

    if(source_length > 0) {

      int length = ::WideCharToMultiByte(codePage, 0, src, (int)source_length, NULL, 0, NULL, NULL);
      if(length == 0) {
        return T();
      }

      std::vector<char> buffer( length );
      ::WideCharToMultiByte(codePage, 0, src, (int)source_length, &buffer[0], length, NULL, NULL);

      return T(buffer.begin(), buffer.end());
    }
    else {
      return T();
    }
  }
  
} // namespace ca

#endif
