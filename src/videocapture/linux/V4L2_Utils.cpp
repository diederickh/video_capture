#include <videocapture/Types.h>
#include <videocapture/linux/V4L2_Utils.h>

namespace ca {

  int capture_format_to_v4l2_pixel_format(int fmt) {
    switch(fmt) {
      case CA_RGB24:       return V4L2_PIX_FMT_RGB24;
      case CA_YUYV422:     return V4L2_PIX_FMT_YUYV;
      case CA_YUV420P:     return V4L2_PIX_FMT_YUV420;
      case CA_YUV422P:     return V4L2_PIX_FMT_YUV422P;
      case CA_H264:        return V4L2_PIX_FMT_H264;
      case CA_MJPEG:       return V4L2_PIX_FMT_MJPEG;
      default:             return CA_NONE;
    }
  }

  int v4l2_pixel_format_to_capture_format(int fmt) {
    switch(fmt) {
      case V4L2_PIX_FMT_RGB24:           return CA_RGB24;
      case V4L2_PIX_FMT_YUYV:            return CA_YUYV422;
      case V4L2_PIX_FMT_YUV420:          return CA_YUV420P;
      case V4L2_PIX_FMT_YUV422P:         return CA_YUV422P; 
      case V4L2_PIX_FMT_H264:            return CA_H264; 
      case V4L2_PIX_FMT_MJPEG:           return CA_MJPEG; 
      default:                           return CA_NONE;
    }
  }

  std::string v4l2_pixel_format_to_string(int fmt) {
    switch(fmt) {
      case V4L2_PIX_FMT_RGB332: return "V4L2_PIX_FMT_RGB332"; break;
      case V4L2_PIX_FMT_RGB444: return "V4L2_PIX_FMT_RGB444"; break;
      case V4L2_PIX_FMT_RGB555: return "V4L2_PIX_FMT_RGB555"; break;
      case V4L2_PIX_FMT_RGB565: return "V4L2_PIX_FMT_RGB565"; break;
      case V4L2_PIX_FMT_RGB555X: return "V4L2_PIX_FMT_RGB555X"; break;
      case V4L2_PIX_FMT_RGB565X: return "V4L2_PIX_FMT_RGB565X"; break;
      case V4L2_PIX_FMT_BGR666: return "V4L2_PIX_FMT_BGR666"; break;
      case V4L2_PIX_FMT_BGR24: return "V4L2_PIX_FMT_BGR24"; break;
      case V4L2_PIX_FMT_RGB24: return "V4L2_PIX_FMT_RGB24"; break;
      case V4L2_PIX_FMT_BGR32: return "V4L2_PIX_FMT_BGR32"; break;
      case V4L2_PIX_FMT_RGB32: return "V4L2_PIX_FMT_RGB32"; break;
      case V4L2_PIX_FMT_GREY: return "V4L2_PIX_FMT_GREY"; break;
      case V4L2_PIX_FMT_Y4: return "V4L2_PIX_FMT_Y4"; break;
      case V4L2_PIX_FMT_Y6: return "V4L2_PIX_FMT_Y6"; break;
      case V4L2_PIX_FMT_Y10: return "V4L2_PIX_FMT_Y10"; break;
      case V4L2_PIX_FMT_Y12: return "V4L2_PIX_FMT_Y12"; break;
      case V4L2_PIX_FMT_Y16: return "V4L2_PIX_FMT_Y16"; break;
      case V4L2_PIX_FMT_Y10BPACK: return "V4L2_PIX_FMT_Y10BPACK"; break;
      case V4L2_PIX_FMT_PAL8: return "V4L2_PIX_FMT_PAL8"; break;
      case V4L2_PIX_FMT_YVU410: return "V4L2_PIX_FMT_YVU410"; break;
      case V4L2_PIX_FMT_YVU420: return "V4L2_PIX_FMT_YVU420"; break;
      case V4L2_PIX_FMT_YUYV: return "V4L2_PIX_FMT_YUYV"; break;
      case V4L2_PIX_FMT_YYUV: return "V4L2_PIX_FMT_YYUV"; break;
      case V4L2_PIX_FMT_YVYU: return "V4L2_PIX_FMT_YVYU"; break;
      case V4L2_PIX_FMT_UYVY: return "V4L2_PIX_FMT_UYVY"; break;
      case V4L2_PIX_FMT_VYUY: return "V4L2_PIX_FMT_VYUY"; break;
      case V4L2_PIX_FMT_YUV422P: return "V4L2_PIX_FMT_YUV422P"; break;
      case V4L2_PIX_FMT_YUV411P: return "V4L2_PIX_FMT_YUV411P"; break;
      case V4L2_PIX_FMT_Y41P: return "V4L2_PIX_FMT_Y41P"; break;
      case V4L2_PIX_FMT_YUV444: return "V4L2_PIX_FMT_YUV444"; break;
      case V4L2_PIX_FMT_YUV555: return "V4L2_PIX_FMT_YUV555"; break;
      case V4L2_PIX_FMT_YUV565: return "V4L2_PIX_FMT_YUV565"; break;
      case V4L2_PIX_FMT_YUV32: return "V4L2_PIX_FMT_YUV32"; break;
      case V4L2_PIX_FMT_YUV410: return "V4L2_PIX_FMT_YUV410"; break;
      case V4L2_PIX_FMT_YUV420: return "V4L2_PIX_FMT_YUV420"; break;
      case V4L2_PIX_FMT_HI240: return "V4L2_PIX_FMT_HI240"; break;
      case V4L2_PIX_FMT_HM12: return "V4L2_PIX_FMT_HM12"; break;
      case V4L2_PIX_FMT_M420: return "V4L2_PIX_FMT_M420"; break;
      case V4L2_PIX_FMT_NV12: return "V4L2_PIX_FMT_NV12"; break;
      case V4L2_PIX_FMT_NV21: return "V4L2_PIX_FMT_NV21"; break;
      case V4L2_PIX_FMT_NV16: return "V4L2_PIX_FMT_NV16"; break;
      case V4L2_PIX_FMT_NV61: return "V4L2_PIX_FMT_NV61"; break;
#if defined(V4L2_PIX_FMT_NV24)
      case V4L2_PIX_FMT_NV24: return "V4L2_PIX_FMT_NV24"; break;
#endif
#if defined(V4L2_PIX_FMT_NV42)        
      case V4L2_PIX_FMT_NV42: return "V4L2_PIX_FMT_NV42"; break;
#endif        
      case V4L2_PIX_FMT_NV12M: return "V4L2_PIX_FMT_NV12M"; break;
#if defined(V4L2_PIX_FMT_NV21M)
      case V4L2_PIX_FMT_NV21M: return "V4L2_PIX_FMT_NV21M"; break;
#endif
#if defined(V4L2_PIX_FMT_NV12MT)
      case V4L2_PIX_FMT_NV12MT: return "V4L2_PIX_FMT_NV12MT"; break;
#endif
#if defined(V4L2_PIX_FMT_NV12MT_16X16)
      case V4L2_PIX_FMT_NV12MT_16X16: return  "V4L2_PIX_FMT_NV12MT_16X16"; break;
#endif
#if defined(V4L2_PIX_FMT_YUV420M)
      case V4L2_PIX_FMT_YUV420M: return "V4L2_PIX_FMT_YUV420M"; break;
#endif
#if defined(V4L2_PIX_FMT_YVU420M)
      case V4L2_PIX_FMT_YVU420M: return "V4L2_PIX_FMT_YVU420M"; break;
#endif
      case V4L2_PIX_FMT_SBGGR8: return "V4L2_PIX_FMT_SBGGR8"; break;
      case V4L2_PIX_FMT_SGBRG8: return "V4L2_PIX_FMT_SGBRG8"; break;
      case V4L2_PIX_FMT_SGRBG8: return "V4L2_PIX_FMT_SGRBG8"; break;
      case V4L2_PIX_FMT_SRGGB8: return "V4L2_PIX_FMT_SRGGB8"; break;
      case V4L2_PIX_FMT_SBGGR10: return "V4L2_PIX_FMT_SBGGR10"; break;
      case V4L2_PIX_FMT_SGBRG10: return "V4L2_PIX_FMT_SGBRG10"; break;
      case V4L2_PIX_FMT_SGRBG10: return "V4L2_PIX_FMT_SGRBG10"; break;
      case V4L2_PIX_FMT_SRGGB10: return "V4L2_PIX_FMT_SRGGB10"; break;
      case V4L2_PIX_FMT_SBGGR12: return "V4L2_PIX_FMT_SBGGR12"; break;
      case V4L2_PIX_FMT_SGBRG12: return "V4L2_PIX_FMT_SGBRG12"; break;
      case V4L2_PIX_FMT_SGRBG12: return "V4L2_PIX_FMT_SGRBG12"; break;
      case V4L2_PIX_FMT_SRGGB12: return "V4L2_PIX_FMT_SRGGB12"; break;
#if defined(V4L2_PIX_FMT_SBGGR10DPCM8)        
      case V4L2_PIX_FMT_SBGGR10DPCM8: return "V4L2_PIX_FMT_SBGGR10DPCM8"; break;
#endif
#if defined(V4L2_PIX_FMT_SGBRG10DPCM8)        
      case V4L2_PIX_FMT_SGBRG10DPCM8: return "V4L2_PIX_FMT_SGBRG10DPCM8"; break;
#endif
#if defined(V4L2_PIX_FMT_SGRBG10DPCM8)        
      case V4L2_PIX_FMT_SGRBG10DPCM8: return "V4L2_PIX_FMT_SGRBG10DPCM8"; break;
#endif
#if defined(V4L2_PIX_FMT_SRGGB10DPCM8)        
      case V4L2_PIX_FMT_SRGGB10DPCM8: return "V4L2_PIX_FMT_SRGGB10DPCM8"; break;
#endif        
      case V4L2_PIX_FMT_SBGGR16: return "V4L2_PIX_FMT_SBGGR16"; break;
      case V4L2_PIX_FMT_MJPEG: return "V4L2_PIX_FMT_MJPEG"; break;
      case V4L2_PIX_FMT_JPEG: return "V4L2_PIX_FMT_JPEG"; break;
      case V4L2_PIX_FMT_DV: return "V4L2_PIX_FMT_DV"; break;
      case V4L2_PIX_FMT_MPEG: return "V4L2_PIX_FMT_MPEG"; break;
      case V4L2_PIX_FMT_H264: return "V4L2_PIX_FMT_H264"; break;
      case V4L2_PIX_FMT_H264_NO_SC:  return "V4L2_PIX_FMT_H264_NO_SC"; break;
#if defined(V4L2_PIX_FMT_H264_MVC)
      case V4L2_PIX_FMT_H264_MVC: return "V4L2_PIX_FMT_H264_MVC"; break; 
#endif
      case V4L2_PIX_FMT_H263: return "V4L2_PIX_FMT_H263"; break;
      case V4L2_PIX_FMT_MPEG1: return "V4L2_PIX_FMT_MPEG1"; break;
      case V4L2_PIX_FMT_MPEG2: return "V4L2_PIX_FMT_MPEG2"; break;
      case V4L2_PIX_FMT_MPEG4: return "V4L2_PIX_FMT_MPEG4"; break;
      case V4L2_PIX_FMT_XVID: return "V4L2_PIX_FMT_XVID"; break;
      case V4L2_PIX_FMT_VC1_ANNEX_G: return "V4L2_PIX_FMT_VC1_ANNEX_G"; break; 
      case V4L2_PIX_FMT_VC1_ANNEX_L: return "V4L2_PIX_FMT_VC1_ANNEX_L"; break; 
#if defined(V4L2_PIX_FMT_VP8)
      case V4L2_PIX_FMT_VP8: return "V4L2_PIX_FMT_VP8"; break;
#endif
      case V4L2_PIX_FMT_CPIA1: return "V4L2_PIX_FMT_CPIA1"; break;
      case V4L2_PIX_FMT_WNVA: return "V4L2_PIX_FMT_WNVA"; break;
      case V4L2_PIX_FMT_SN9C10X: return "V4L2_PIX_FMT_SN9C10X"; break;
      case V4L2_PIX_FMT_SN9C20X_I420: return "V4L2_PIX_FMT_SN9C20X_I420"; break; 
      case V4L2_PIX_FMT_PWC1: return "V4L2_PIX_FMT_PWC1"; break;
      case V4L2_PIX_FMT_PWC2: return "V4L2_PIX_FMT_PWC2"; break;
      case V4L2_PIX_FMT_ET61X251: return "V4L2_PIX_FMT_ET61X251"; break;
      case V4L2_PIX_FMT_SPCA501: return "V4L2_PIX_FMT_SPCA501"; break;
      case V4L2_PIX_FMT_SPCA505: return "V4L2_PIX_FMT_SPCA505"; break;
      case V4L2_PIX_FMT_SPCA508: return "V4L2_PIX_FMT_SPCA508"; break;
      case V4L2_PIX_FMT_SPCA561: return "V4L2_PIX_FMT_SPCA561"; break;
      case V4L2_PIX_FMT_PAC207: return "V4L2_PIX_FMT_PAC207"; break;
      case V4L2_PIX_FMT_MR97310A: return "V4L2_PIX_FMT_MR97310A"; break;
#if defined(V4L2_PIX_FMT_JL2005BCD)        
      case V4L2_PIX_FMT_JL2005BCD: return "V4L2_PIX_FMT_JL2005BCD"; break;
#endif        
      case V4L2_PIX_FMT_SN9C2028: return "V4L2_PIX_FMT_SN9C2028"; break;
      case V4L2_PIX_FMT_SQ905C: return "V4L2_PIX_FMT_SQ905C"; break;
      case V4L2_PIX_FMT_PJPG: return "V4L2_PIX_FMT_PJPG"; break;
      case V4L2_PIX_FMT_OV511: return "V4L2_PIX_FMT_OV511"; break;
      case V4L2_PIX_FMT_OV518: return "V4L2_PIX_FMT_OV518"; break;
      case V4L2_PIX_FMT_STV0680: return "V4L2_PIX_FMT_STV0680"; break;
      case V4L2_PIX_FMT_TM6000: return "V4L2_PIX_FMT_TM6000"; break;
      case V4L2_PIX_FMT_CIT_YYVYUY: return "V4L2_PIX_FMT_CIT_YYVYUY v4l2"; break; 
      case V4L2_PIX_FMT_KONICA420: return "V4L2_PIX_FMT_KONICA420"; break;
      case V4L2_PIX_FMT_JPGL: return "V4L2_PIX_FMT_JPGL"; break;
      case V4L2_PIX_FMT_SE401: return "V4L2_PIX_FMT_SE401"; break;
#if defined(V4L2_PIX_FMT_S5C_UYVY_JPG)
      case V4L2_PIX_FMT_S5C_UYVY_JPG: return "V4L2_PIX_FMT_S5C_UYVY_JPG"; break; 
#endif
      default: return "UNKNOWN PIXEL FORMAT";  break;
    }
  }

}; // namespace cs
