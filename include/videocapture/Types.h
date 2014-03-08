#ifndef VIDEO_CAPTURE_TYPES_H
#define VIDEO_CAPTURE_TYPES_H

#include <vector>
#include <string>

/* General */
#define CA_NONE -1

/* Capture Drivers */
#define CA_MEDIA_FOUNDATION 1                                                       /* Windows: Capture using Windows Media Foundation. */
#define CA_AV_FOUNDATION 2                                                          /* Mac:     Capture using AVFoundation. */
#define CA_V4L2 3                                                                   /* Linux:   Capture using Video4Linux 2. */

/* Default driver per OS */
#if defined(__APPLE__)
#  define CA_DEFAULT_DRIVER CA_AV_FOUNDATION
#elif defined(__linux)
#  define CA_DEFAULT_DRIVER CA_V4L2
#elif defined(_WIN32)
#  define CA_DEFAULT_DRIVER CA_MEDIA_FOUNDATION
#else 
#  error "We have no default capture implementation for this OS."
#endif

/* Pixel Formats */
#define CA_UYVY422 1                                                                /* Cb Y0 Cr Y1 */
#define CA_YUYV422 2                                                                /* Y0 Cb Y1 Cr */
#define CA_YUV422P 3                                                                /* YUV422 Planar */
#define CA_YUV420P 4                                                                /* YUV420 Planar */
#define CA_YUV420BP 5                                                               /* YUV420 Bi Planar */
#define CA_YUVJ420P 6                                                               /* YUV420 Planar Full Range (JPEG), J comes from the JPEG. (values 0-255 used) */
#define CA_YUVJ420BP 7                                                              /* YUV420 Bi-Planer Full Range (JPEG), J comes fro the JPEG. (values: luma = [16,235], chroma=[16,240]) */
#define CA_ARGB32 8                                                                 /* ARGB 8:8:8:8 32bpp, ARGBARGBARGB... */
#define CA_BGRA32 9                                                                 /* BGRA 8:8:8:8 32bpp, BGRABGRABGRA... */
#define CA_RGB24 10                                                                 /* RGB 8:8:8 24bit */
#define CA_JPEG_OPENDML 11                                                          /* JPEG with Open-DML extensions */
#define CA_H264 12                                                                  /* H264 */
#define CA_MJPEG 13                                                                 /* MJPEG 2*/

/* Frame rates */
#define CA_FPS_60_00   6000
#define CA_FPS_30_00   3000
#define CA_FPS_27_50   2750
#define CA_FPS_25_00   2500
#define CA_FPS_24_00   2400
#define CA_FPS_22_50   2250
#define CA_FPS_20_00   2000
#define CA_FPS_17_50   1750
#define CA_FPS_15_00   1500
#define CA_FPS_12_50   1250
#define CA_FPS_10_00   1000
#define CA_FPS_7_50    750
#define CA_FPS_5_00    500 
#define CA_FPS_2_00    200

/* States (may be be used by implementations) */

#define CA_STATE_NONE 0x00                                                         /* Default state */
#define CA_STATE_OPENED 0x01                                                       /* The user opened a device */
#define CA_STATE_CAPTUREING 0x02                                                   /* The user started captureing */
 
namespace ca {

  typedef void(*frame_callback)(void* pixels, int nbytes, void* user);

  /* -------------------------------------- */

  class Capability {                                                                /* Capability represents a possibility for a capture device. It often is related to a width/height/fps. */
  public:
    Capability();
    ~Capability();
    void clear();                                                                   /* Resets all members to defaults. */

  public:
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
  };

  /* -------------------------------------- */

  class Device {                                                                    /* A device represents a capture device for the implementation. */
  public:
    Device();
    ~Device();
    void clear();                                                                   /* Resets all members to defaults. */
  public:
    int index;                                                                      /* Used by the implementation. */
    std::string name;                                                               /* Name of the device. */
  };

  /* -------------------------------------- */


  class Format {                                                                    /* A format is used to describe an output format into which the raw buffers from the webcam can be converted. This is often a feature from the OS, like the AVCaptureVideoDataOutput. */
  public:
    Format();                                                                       /* Constructors; clears members. */ 
    ~Format();                                                                      /* D'tor, clears members. */
    void clear();

  public:
    int format;                                                                     /* The supported format, one of the CA_UYV*, CA_YUV*, CA_ARGB, etc.. formats. */
    int index;                                                                      /* Index to be used by the implementation. */
  };

  /* -------------------------------------- */

  class Settings {                                                                  /* A settings object is used to open a capture device. It describes what capability, device and output format (if any) you want to use. */
  public:
    Settings();                                                                     /* C'tor; will reset all settings. */
    ~Settings();                                                                    /* D'tor; will reset all settings. */
    void clear();                                                                   /* Clear all settings. */

  public: 
    int capability;                                                                 /* Number of the capability you want to use. See listCapabilities(). */
    int device;                                                                     /* Number of the device you want to use. See listDevices(). */
    int format;                                                                     /* Number of the output format you want to use. See listOutputFormats(). */
  };

  /* -------------------------------------- */

  class Frame {                                                                      /* A frame object can be used to store information about the frame data for a specific pixel format. */
  public:
    Frame();
    ~Frame();
    void clear();
    int set(int w, int h, int fmt);                                                  /* Set the format. This will set all members. */

  public:
    std::vector<int> width;                                                          /* Height per plane */
    std::vector<int> height;                                                         /* Width per plane */
    std::vector<int> stride;                                                         /* Stride per plane */
    std::vector<int> nbytes;                                                         /* Number of bytes per plane */
    std::vector<int> offset;                                                         /* Number of bytes into pixel data when the frame data is continuous */
  };

  /* -------------------------------------- */

}; // namespace ca

#endif
