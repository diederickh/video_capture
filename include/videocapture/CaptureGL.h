/*

  CaptureGL
  ----------

  The webcam class is an simple wrapper around the functionalities of the capture 
  library that takes care of all the lower level things like finding the best pixel 
  format and drawing the frames to screen etc..

  Header only
  -----------
  The `CaptureGL` class is header only class. We're using a header only class so you 
  can include the openGL headers using your own wrapper or system dependent GL file.
  In one file, we recommend main.cpp, define VIDEOCAPTURE_IMPLEMENTATION before you 
  include the <videocapture/CaptureGL.h>. 


  Usage
  -----
  
  ````c++

      // include here your GL headers (e.g. using GLFW)
      #define VIDEO_CAPTURE_IMPLEMENTATION
      #include <videocapture/CaptureGL.h>  

      // ...
      
      ca::CaptureGL capture;
      
      if(capture.open(0, 640, 480) < 0) {
        printf("Error: Can't open the capture device.\n");
        ::exit(EXIT_FAILURE);
      }

      if(capture.start() < 0) {
        printf("Error: Can't start the capture.\n");
        ::exit(EXIT_FAILURE);
      }
    
      while(rendering) {
        capture.update();
        capture.draw(10, 10, 320, 240);
      }
    
      capture.stop();
      capture.close(); 
      
     // ...

  ````      

*/

#ifndef VIDEO_CAPTURE_GL_H
#define VIDEO_CAPTURE_GL_H

#define CA_RENDERING_TYPE_NONE 0
#define CA_RENDERING_TYPE_UYVY422_APPLE 1       /* Uses the GL_RGB_422_APPLE extension. */
#define CA_RENDERING_TYPE_YUYV422_APPLE 2       /* Uses the GL_RGB_422_APPLE extension. */
#define CA_RENDERING_TYPE_UYVY422_GENERIC 3     /* Cross platform UYVY422. Does not use extensions. */
#define CA_RENDERING_TYPE_YUYV422_GENERIC 4     /* Cross platform YUVY422. Does not use extensions. */
#define CA_RENDERING_TYPE_YUV420P_GENERIC 5     /* Cross platform YUV420P rendering. Does not use extensions. */

/* Set GL_RGB422_APPLE extension info */
#ifndef GL_RGB_422_APPLE
#  define GL_RGB_422_APPLE 0x8A1F
#endif

#ifndef GL_UNSIGNED_SHORT_8_8_APPLE
#  define GL_UNSIGNED_SHORT_8_8_APPLE 0x85BA
#endif

#ifndef GL_UNSIGNED_SHORT_8_8_REV_APPLE
#  define GL_UNSIGNED_SHORT_8_8_REV_APPLE 0x85BB
#endif

#ifndef GL_RGB_RAW_422_APPLE
#  define GL_RGB_RAW_422_APPLE 0x8A51
#endif

#include <videocapture/Utils.h>
#include <videocapture/Types.h>
#include <videocapture/Capture.h>
#include <videocapture/CapabilityFinder.h>

#if defined(_WIN32)
   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>
#elif defined(__linux) or defined(__APPLE__)
   #include <pthread.h>
#endif

static const char* CAPTURE_GL_VS = ""
  "#version 330\n"
  "uniform vec4 u_pos; " 
  "uniform float u_texcoords[8]; "
  ""
  "const vec2 pos[4] = vec2[4]("
  "  vec2(0.0,  1.0), "
  "  vec2(0.0, 0.0), "
  "  vec2( 1.0,  1.0), "
  "  vec2( 1.0, 0.0)  "
  ");"
  ""
  "out vec2 v_texcoord; "
  ""
  /*
    u_pos.x = X position in percentage of viewport 0-1. 0% means -1.0
    u_pos.y = Y position in percentage of viewport 0-1. 0% means -1.0
    u_pos.z = WIDTH scale in percentage of viewport. 
    u_pos.w = HEIGHT scale in percentage of viewport. 
   */
  "void main() { "
  "  vec2 p = pos[gl_VertexID]; " 
  "  vec2 offset = vec2(u_pos.x * 2.0 - 1.0, u_pos.y * 2.0 - 1.0); "
  "  vec2 scale  = vec2(u_pos.z * p.x * 2.0, u_pos.w * p.y * 2.0); " 
  "  gl_Position = vec4(offset.x + scale.x, "
  "                     offset.y + scale.y, "
  "                     0.0, 1.0);"
  " v_texcoord = vec2(u_texcoords[gl_VertexID * 2], u_texcoords[gl_VertexID * 2 + 1]);"
  "}"
  "";

/* Uses the APPLE RGB 422 extension: note that this is identical as `CAPTURE_GL_YUVY422_APPLE_FS`, we use GL_UNSIGNED_SHORT_8_8_APPLE or YUYV and GL_UNSIGNED_SHORT_8_8_REV for UYVY, see updateYUYV422(). */ 
static const char* CAPTURE_GL_UYVY422_APPLE_FS = ""
  "#version 330\n"
  "uniform sampler2D u_tex;"
  "layout( location = 0 ) out vec4 outcol; "

  "const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);"
  "const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);"
  "const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);"
  "const vec3 offset = vec3(-0.0625, -0.5, -0.5);"
 
  "in vec2 v_texcoord;"
  ""
  "void main() {"
  "  vec3 tc =  texture( u_tex, v_texcoord ).rgb;"
  "  vec3 yuv = vec3(tc.g, tc.b, tc.r);"
  "  yuv += offset;"
  "  outcol.r = dot(yuv, R_cf);"
  "  outcol.g = dot(yuv, G_cf);"
  "  outcol.b = dot(yuv, B_cf);"
  "  outcol.a = 1.0;"
  "}"
  "";

/* Uses the APPLE RGB 422 extension, for YUYV422: note that this is identical as `CAPTURE_GL_UYVY422_APPLE_FS`, we use GL_UNSIGNED_SHORT_8_8_APPLE or YUYV and GL_UNSIGNED_SHORT_8_8_REV for UYVY, see updateYUYV422(). */
static const char* CAPTURE_GL_YUYV422_APPLE_FS = ""
  "#version 330\n"
  "uniform sampler2D u_tex;"
  "layout( location = 0 ) out vec4 outcol; "

  "const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);"
  "const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);"
  "const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);"
  "const vec3 offset = vec3(-0.0625, -0.5, -0.5);"
 
  "in vec2 v_texcoord;"
  ""
  "void main() {"
  "  vec3 tc =  texture( u_tex, v_texcoord ).rgb;"
  "  vec3 yuv = vec3(tc.g, tc.b, tc.r);"
  "  yuv += offset;"
  "  outcol.r = dot(yuv, R_cf);"
  "  outcol.g = dot(yuv, G_cf);"
  "  outcol.b = dot(yuv, B_cf);"
  "  outcol.a = 1.0;"
  "}"
  "";

/* Generic UYVY422 */
static const char* CAPTURE_GL_UYVY422_GENERIC_FS = ""
  "#version 330\n"
  ""
  "uniform sampler2D u_tex;"
  "layout( location = 0 ) out vec4 fragcolor; "
  "in vec2 v_texcoord;"
  ""
  "const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);"
  "const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);"
  "const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);"
  "const vec3 offset = vec3(-0.0625, -0.5, -0.5);"
  ""
  "void main() {"
  ""
  "  int width = textureSize(u_tex, 0).x * 2;"
  "  float tex_x = v_texcoord.x; "
  "  int pixel = int(floor(width * tex_x)) % 2; "
  "  vec4 tc =  texture( u_tex, v_texcoord ).rgba;"
  ""
  "  float cr = tc.r; "
  "  float y1 = tc.g; "
  "  float cb = tc.b; "
  "  float y2 = tc.a; "
  ""
  "  float y = (pixel == 1) ? y2 : y1; "
  "  vec3 yuv = vec3(y, cb, cr);"
  "  yuv += offset;"
  ""
  "  fragcolor.r = dot(yuv, R_cf);"
  "  fragcolor.g = dot(yuv, G_cf);"
  "  fragcolor.b = dot(yuv, B_cf);"
  "  fragcolor.a = 1.0;"
  "}"
  "";

/* Generic YUYV422, or YUV 4:2:2 (same thing, different names) shader */
static const char* CAPTURE_GL_YUYV422_GENERIC_FS = ""
  "#version 330\n"
  ""
  "uniform sampler2D u_tex;"
  "layout( location = 0 ) out vec4 fragcolor; "
  "in vec2 v_texcoord;"
  ""
  "const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);"
  "const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);"
  "const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);"
  "const vec3 offset = vec3(-0.0625, -0.5, -0.5);"
  ""
  "void main() {"
  ""
  "  int width = textureSize(u_tex, 0).x * 2;"
  "  float tex_x = v_texcoord.x; "
  "  int pixel = int(floor(width * tex_x)) % 2; "
  "  vec4 tc =  texture( u_tex, v_texcoord ).rgba;"
  ""
  "  float cr = tc.a; "
  "  float y2 = tc.b; "
  "  float cb = tc.g; "
  "  float y1 = tc.r; "
  ""
  "  float y = (pixel == 1) ? y2 : y1; "
  "  vec3 yuv = vec3(y, cb, cr);"
  "  yuv += offset;"
  ""
  "  fragcolor.r = dot(yuv, R_cf);"
  "  fragcolor.g = dot(yuv, G_cf);"
  "  fragcolor.b = dot(yuv, B_cf);"
  "  fragcolor.a = 1.0;"
  "}"
  "";

/* Decode YUV420P (3 planes) */
static const char* CAPTURE_GL_YUV420P_GENERIC_FS = ""
  "#version 330\n"
  "uniform sampler2D y_tex;"
  "uniform sampler2D u_tex;"
  "uniform sampler2D v_tex;"
  "in vec2 v_texcoord;"
  "layout( location = 0 ) out vec4 fragcolor;"
  ""
  "const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);"
  "const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);"
  "const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);"
  "const vec3 offset = vec3(-0.0625, -0.5, -0.5);"
  ""
  "void main() {"
  "  float y = texture(y_tex, v_texcoord).r;"
  "  float u = texture(u_tex, v_texcoord).r;"
  "  float v = texture(v_tex, v_texcoord).r;"
  "  vec3 yuv = vec3(y,u,v);"
  "  yuv += offset;"
  "  fragcolor = vec4(0.0, 0.0, 0.0, 1.0);"
  "  fragcolor.r = dot(yuv, R_cf);"
  "  fragcolor.g = dot(yuv, G_cf);"
  "  fragcolor.b = dot(yuv, B_cf);"
  "}"
  "";

namespace ca {

  void capturegl_on_frame(PixelBuffer& buffer);                                       /* Is called whenever a frame is received from the capture device */
  GLuint capturegl_create_shader(GLenum type, const char* source);                    /* Creates a shader for the given type. */

  /* Threading, Mutex. */
#if defined(_WIN32)
  struct CaptureMutex {
    CRITICAL_SECTION handle;
  };
#elif defined(__linux) or defined(__APPLE__)
  struct CaptureMutex {
    pthread_mutex_t handle;
  };
#endif
  
  class CaptureGL {
    
  public:
    CaptureGL(int driver = CA_DEFAULT_DRIVER);
    ~CaptureGL();
    int open(int device, int width, int height);                                       /* Setup the capturer for the given device and dimensions. We will try to find the most optimal pixel format format. This function will return -1 on error, else the same as the given divice */
    int open(Settings cfg);                                                            /* Setup the capturer using the given settings (e.g. using the given capability id) . */
    int close();                                                                       /* Close the capture device */
    int start();                                                                       /* Start capturing */ 
    int stop();                                                                        /* Stop capturing */
    int update();                                                                      /* Call this regularly. Returns 1 when we update pixels, -1 on error and else 0. */
    void resize(int w, int h);                                                         /* Call w/h when the viewport resizes */
    void draw();                                                                       /* Draw, filling the the viewport */
    void draw(int x, int y, int w, int h);                                             /* Draw the current frame at x,y and w/h */
    void flip(bool horizontal, bool vertical);                                         /* Flip the video horizontally or vertically. Following photoshop conventions. */

    /* Capabilities */
    //    int getOutputFormat();                                                             /* Get the output format that is used for the pixel buffers that are passed into the callback. This should only be called wen you've opened the capture device. */
    std::vector<Capability> getCapabilities(int device);                               /* Get the capabilities for the given device. */
    std::vector<Device> getDevices();                                                  /* Get a vector with the devices. */
    std::vector<Format> getOutputFormats();                                            /* Get the output formats that we can use. */ 
    
    /* Info */
    int listDevices();                                                                 /* Wrapper around Capture::listDevices(). */
    int listCapabilities(int device);                                                  /* Wrapper around Capture::listCapabilities(). */
    int listOutputFormats();                                                           /* Wrapper around Capture::listOutputFormats(). */
    int findCapability(int device, int width, int height, int fmt);                    /* Wrapper around Capture::findCapability() */

    void setRenderingType(int rtype);                                                  /* Set one of the rendering types, is used to determine which shader to use. */
    const char* getFragmentShaderSource();                                             /* Returns the fragment shader to use, based on the current rendering type. */

    /* Threading. */
    int createMutex(CaptureMutex& m);
    int destroyMutex(CaptureMutex& m); 
    int lockMutex(CaptureMutex& m);
    int unlockMutex(CaptureMutex& m);

  private:
    int setupGraphics();                                                               /* Creates the opengl objects */
    int setupShaders();                                                                /* Creates shaders for the pixel format */
    int setupTextures();                                                               /* Creates the textures that back the current pixel format */
    void updateYUYV422();                                                              /* Updates the YUYV422 pixel data */
    void updateYUV420P();

  public:
    int rendering_type;                                                                /* what rendering type to use; e.g. use APPLE extension, or a generic YUV422 shader. */ 
    int width;                                                                         /* The width of the frames we capture */
    int height;                                                                        /* The height of the frames we capture */
    float inv_win_w;                                                                   /* Inverse window/viewport width, used to calculate the correct position/offset values for the vertex shader . */
    float inv_win_h;                                                                   /* Inverse window/viewport height, used to calculate the correct position/offset values for the vertex shader . */
    int win_w;
    int win_h;
    int pixel_format;                                                                  /* The best matching pixel format that we found and which is used. */ 
    Frame frame;                                                                       /* The `Frame` object is used to get information about width/height/bytes used for the current pixel format */ 
    Capture cap;                                                                       /* We use the Capture class for all capture related things */
    GLuint frag;                                                                       /* The fragment shader that does the conversion */
    GLuint vert;                                                                       /* Simple vertex shader using attribute less rendering */
    GLuint prog;                                                                       /* The shader program */  
    GLuint vao;                                                                        /* We need a VAO to render attribute-less */
    GLint u_pos;                                                                       /* Points to the u_pos in the vertex shader; used to scale and offset the vertex position */
    GLint u_texcoords;                                                                 /* Offset into the texcoord array in the shader to flip vertically and horizontally */
    GLint u_tex0;                                                                      /* Points to the first texture element in the frag shader. */
    GLint u_tex1;                                                                      /* Points to the second plane (e.g. when format is YUV420P) */
    GLint u_tex2;                                                                      /* Points to the thirds plane (e.g. when format is YUV420P) */
    GLuint tex0;                                                                       /* Texture that will be filled with the pixel data from the webcam */
    GLuint tex1;                                                                       /* Texture that will be filled with the pixel data from the webcam (e.g. the U plane when using YUV420P). */
    GLuint tex2;                                                                       /* Texture that will be filled with the pixel data from the webcam (e.g. the U plane when using YUV420P). */
    unsigned char* pixels;                                                             /* When we use the YUYV422/YUV420 this will hold all the data */
    bool needs_update;                                                                 /* Is set to true when we receive a new frame */
    CaptureMutex mutex;                                                                /* We use a mutex when accessing the pixel buffer we receive from the capture thread. */ 
  };

  inline void CaptureGL::setRenderingType(int rtype) {
    rendering_type = rtype;
  }

  // Update the YUYV422 Pixels
  inline void CaptureGL::updateYUYV422() {
    
    glBindTexture(GL_TEXTURE_2D, tex0);

    if (rendering_type == CA_RENDERING_TYPE_YUYV422_APPLE) {
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, (uint16_t*)pixels);
    }
    else if (rendering_type == CA_RENDERING_TYPE_UYVY422_APPLE) {
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, (uint16_t*)pixels);
    }
    else if (rendering_type == CA_RENDERING_TYPE_YUYV422_GENERIC
             || rendering_type == CA_RENDERING_TYPE_UYVY422_GENERIC)
    {
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[0], frame.height[0], GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
    }
    else {
      printf("Unhandled pixel format in updateYUYV.\n");
    }
  }

  /* Update the YUV420P Pixels */
  inline void CaptureGL::updateYUV420P() {
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glBindTexture(GL_TEXTURE_2D, tex0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[0], frame.height[0], GL_RED, GL_UNSIGNED_BYTE, pixels + frame.offset[0]);

    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[1], frame.height[1], GL_RED, GL_UNSIGNED_BYTE, pixels + frame.offset[1]);

    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[2], frame.height[2], GL_RED, GL_UNSIGNED_BYTE, pixels + frame.offset[2]);
  }

  /* Some wrappers around ca::Capture */
  inline int CaptureGL::listDevices() {
    return cap.listDevices();
  }
                         
  inline int CaptureGL::listCapabilities(int device) {
    return cap.listCapabilities(device);
  }          

  inline int CaptureGL::listOutputFormats() {
    return cap.listOutputFormats();
  }                   

  inline int CaptureGL::findCapability(int device, int width, int height, int fmt) {
    return cap.findCapability(device, width, height, fmt);
  }
  
} /* namespace ca */

#endif

#if defined(VIDEO_CAPTURE_IMPLEMENTATION)

namespace ca {

  /* -------------------------------------- */

  static int print_shader_compile_info(GLuint shader); 
  static int print_program_link_info(GLuint prog);
  static int capturegl_is_extension_supported(std::string name);

  /* -------------------------------------- */

  void capturegl_on_frame(PixelBuffer& pixbuf) { 

    if(pixbuf.nbytes == 0) {
      return;
    }

    CaptureGL* gl = static_cast<CaptureGL*>(pixbuf.user);

    if (pixbuf.pixel_format == CA_YUYV422
       || pixbuf.pixel_format == CA_UYVY422
       || pixbuf.pixel_format == CA_YUV420P)
    {
      gl->lockMutex(gl->mutex);
      {
        memcpy((char*)gl->pixels, (char*)pixbuf.plane[0], pixbuf.nbytes);
        gl->needs_update = true;
      }
      gl->unlockMutex(gl->mutex);
    }
    else {
      printf("Error: pixels not handled in capturegl_on_frame(), format: %s.\n", format_to_string(pixbuf.pixel_format).c_str());
    }
  }

  GLuint capturegl_create_shader(GLenum type, const char* source) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &source, NULL);
    glCompileShader(s);
    print_shader_compile_info(s);
    return s;
  }

  /* -------------------------------------- */

  CaptureGL::CaptureGL(int driver)
    :cap(capturegl_on_frame, this, driver)
    ,rendering_type(CA_RENDERING_TYPE_NONE)
    ,width(0)
    ,height(0)
    ,inv_win_w(0)
    ,inv_win_h(0)
    ,win_w(0)
    ,win_h(0)
    ,pixel_format(CA_NONE)
    ,frag(0)
    ,vert(0)
    ,prog(0)
    ,vao(0)
    ,u_pos(0)
    ,u_texcoords(0)
    ,u_tex0(0)
    ,u_tex1(0)
    ,u_tex2(0)
    ,tex0(0)
    ,tex1(0)
    ,tex2(0)
    ,pixels(NULL)
    ,needs_update(false)
    {
      createMutex(mutex);
    }

  CaptureGL::~CaptureGL() {

    if(pixels) {
      delete[] pixels;
      pixels = NULL;
    }

    needs_update = false;
    width = 0;
    height = 0;
    inv_win_w = 0.0f;
    inv_win_h = 0.0f;
    u_pos = 0;
    u_texcoords = 0;
    u_tex0 = 0;
    u_tex1 = 0;
    u_tex2 = 0;

    destroyMutex(mutex);
    
    /* @todo - free GL  */
  }

  int CaptureGL::open(int device, int w, int h) {

    Settings settings;
    float ratio = float(w) / h;
    CapabilityFinder finder(cap);
    
    finder.addFilter(CA_PIXEL_FORMAT, CA_YUYV422, 100);
    finder.addFilter(CA_PIXEL_FORMAT, CA_UYVY422, 100);
    finder.addFilter(CA_PIXEL_FORMAT, CA_YUV420P, 100);
    finder.addFilter(CA_WIDTH, w, 95);
    finder.addFilter(CA_HEIGHT, h, 95);
    finder.addFilter(CA_RATIO, ratio, 90);

    int supported_formats[] = { CA_YUYV422, CA_UYVY422, CA_YUV420P };
    bool found_format = false;
    for (int i = 0; i < 3; ++i) {
      if (0 == finder.findSettingsForFormat(device, supported_formats[i], settings)) {
        found_format = true;
        pixel_format = supported_formats[i];
        break;
      }
    }

    if (false == found_format) {
      printf("Error: failed to find a capability for the given width/height and supported formats.\n");
      return -1;
    }

    if (open(settings) < 0) {
      return -2;
    }

    return device;
  }

  int CaptureGL::open(Settings cfg) {

    if (CA_NONE == cfg.capability) {
      printf("Error: trying to open the CaptureGL with a Settings object which doesn't have a capability set.\n");
      return -1;
    }

    if (CA_NONE == cfg.device) {
      printf("Error: not device id given, cannot open capture device.\n");
      return -2;
    }

    std::vector<Capability> caps = getCapabilities(cfg.device);
    if (cfg.capability >= caps.size()) {
      printf("Error: the set capability index is out of bounds.\n");
      return -3;
    }

    Capability& found_cap = caps[cfg.capability];
    width = found_cap.width;
    height = found_cap.height;

    if (CA_NONE != cfg.format) {
      pixel_format = cfg.format;
    }
    else {
      pixel_format = found_cap.pixel_format;
    }

    printf("Using format: %s, capability: %d\n", format_to_string(pixel_format).c_str(), cfg.capability);

    if(cap.open(cfg) < 0) {
      printf("Error: cannot open the capture device.\n");
      return -3;
    }

    /* After opening we need to update the format because some SDKs can convert the format. */

    if(setupGraphics() < 0) {
      printf("Error: cannot setup the GL graphics.\n");
      return -4;
    }

    /* set the scale factor. */
    GLint vp[4]; 
    glGetIntegerv(GL_VIEWPORT, vp);
    resize(vp[2], vp[3]);
    
    return 0;
  }

  int CaptureGL::close() {
    return cap.close();
  }

  int CaptureGL::start() {
    return cap.start();
  }

  int CaptureGL::update() {

    bool has_new_frame = false;

    cap.update();

    lockMutex(mutex);
    {
      has_new_frame = needs_update;
    }
    unlockMutex(mutex);

    if (false == has_new_frame) {
      return false;
    }
    
    if (has_new_frame) {

      lockMutex(mutex);
      {
        if (pixel_format == CA_YUV420P) {
          updateYUV420P();
        }
        else if (pixel_format == CA_YUYV422 || pixel_format == CA_UYVY422) {
          updateYUYV422();
        }
        needs_update = false;
      }
      unlockMutex(mutex);
    }

    return needs_update;
  }

  int CaptureGL::stop() {
    return cap.stop();
  }

  std::vector<Device> CaptureGL::getDevices() {
    return cap.getDevices();
  }

  std::vector<Capability> CaptureGL::getCapabilities(int device) {
    return cap.getCapabilities(device);
  }

  std::vector<Format> CaptureGL::getOutputFormats() {
    return cap.getOutputFormats();
  }

  int CaptureGL::setupGraphics() {

    /* Determine the best rendering type. */
    if (0 == capturegl_is_extension_supported("GL_RGB_422_APPLE")) {
      if (pixel_format == CA_UYVY422) {
        setRenderingType(CA_RENDERING_TYPE_UYVY422_APPLE);
      }
      else if (pixel_format == CA_YUYV422) {
        setRenderingType(CA_RENDERING_TYPE_YUYV422_APPLE);
      }
      else {
        printf("Unhandled pixel format in setupGraphics(), %s.\n", format_to_string(pixel_format).c_str());
        exit(1);
      }
    }
    else {
      /* GL_RGB422 is defined, but not supported. */
      if (pixel_format == CA_UYVY422) {
        setRenderingType(CA_RENDERING_TYPE_UYVY422_GENERIC);
      }
      else if (pixel_format == CA_YUYV422) {
        setRenderingType(CA_RENDERING_TYPE_YUYV422_GENERIC);
      }
      else if (pixel_format == CA_YUV420P) {
        setRenderingType(CA_RENDERING_TYPE_YUV420P_GENERIC);
      }
      else {
        printf("Unhandled pixel format in setupGraphics(), %s.\n", format_to_string(pixel_format).c_str());
        exit(1);
      }
    }

    glGenVertexArrays(1, &vao);    
        
    if(setupShaders() < 0) {
      return -1;
    }

    if(setupTextures() < 0) {
      return -2;
    }

    /* set default texture coordinates. */
    flip(false, false);

    return 1;
  }

  const char* CaptureGL::getFragmentShaderSource() {
    switch (rendering_type) {
      case CA_RENDERING_TYPE_YUYV422_GENERIC: {
        return CAPTURE_GL_YUYV422_GENERIC_FS;
      }
      case CA_RENDERING_TYPE_UYVY422_GENERIC: {
        return CAPTURE_GL_UYVY422_GENERIC_FS;
      }
      case CA_RENDERING_TYPE_YUYV422_APPLE: {
        return CAPTURE_GL_YUYV422_APPLE_FS;
      }
      case CA_RENDERING_TYPE_UYVY422_APPLE: {
        return CAPTURE_GL_UYVY422_APPLE_FS;
      }
      case CA_RENDERING_TYPE_YUV420P_GENERIC: {
        return CAPTURE_GL_YUV420P_GENERIC_FS;
      }
      case CA_RENDERING_TYPE_NONE: {
        printf("Error: cannot find rendering type and therefore no shader source.\n");
        return NULL;
      }
    }
    return NULL;
  }

  int CaptureGL::setupShaders() {

    const char* fragment_source = getFragmentShaderSource();
    if (fragment_source == NULL) {
      printf("Error: cannot retrieve a fragment shahder source for the current rendering type.\n");
      return -1;
    }

    vert = capturegl_create_shader(GL_VERTEX_SHADER, CAPTURE_GL_VS);
    frag = capturegl_create_shader(GL_FRAGMENT_SHADER, fragment_source);

    prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    print_program_link_info(prog);

    glUseProgram(prog);

    if(pixel_format == CA_YUV420P) {
      u_tex0 = glGetUniformLocation(prog, "y_tex");
      u_tex1 = glGetUniformLocation(prog, "u_tex");
      u_tex2 = glGetUniformLocation(prog, "v_tex");
      glUniform1i(u_tex0, 0);
      glUniform1i(u_tex1, 1);
      glUniform1i(u_tex2, 2);
    }
    else if(pixel_format == CA_YUYV422 || pixel_format == CA_UYVY422) {
      u_tex0 = glGetUniformLocation(prog, "u_tex");
      glUniform1i(u_tex0, 0);
    }
    
    u_pos = glGetUniformLocation(prog, "u_pos");
    u_texcoords = glGetUniformLocation(prog, "u_texcoords");
   
    glUseProgram(0);
    return 1;
  }

  int CaptureGL::setupTextures() {

    if(frame.set(width, height, pixel_format) < 0) {
      printf("Error: cannot set the frame object.\n");
      return -1;
    }
    
    /* create textures to the back the pixel format. */
    if(pixel_format == CA_YUV420P) {
      pixels = new unsigned char[frame.nbytes[0] + frame.nbytes[1] + frame.nbytes[2]];

      glGenTextures(1, &tex0);
      glBindTexture(GL_TEXTURE_2D, tex0);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, frame.width[0], frame.height[0], 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glGenTextures(1, &tex1);
      glBindTexture(GL_TEXTURE_2D, tex1);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, frame.width[1], frame.height[1], 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glGenTextures(1, &tex2);
      glBindTexture(GL_TEXTURE_2D, tex2);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, frame.width[2], frame.height[2], 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else if(pixel_format == CA_YUYV422 || pixel_format == CA_UYVY422) {

      pixels = new unsigned char[frame.nbytes[0]];
      memset(pixels, 0x00, frame.nbytes[0]);

      glGenTextures(1, &tex0);
      glBindTexture(GL_TEXTURE_2D, tex0);

      /* Even when GL_RGB422_APPLE is found the extension may not be supported. */
      if (rendering_type == CA_RENDERING_TYPE_YUYV422_GENERIC
          || rendering_type == CA_RENDERING_TYPE_UYVY422_GENERIC)
      {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.width[0], frame.height[0], 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
      }
      else if (rendering_type == CA_RENDERING_TYPE_YUYV422_APPLE
               || rendering_type == CA_RENDERING_TYPE_UYVY422_APPLE)
      {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, pixels);
      }
      else {
        printf("Error: unhandled pixel format, cannot create a texture.\n");
        return -1;
      }

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    }
    else {
      printf("Error: unhandled pixel format in CaptureGL.\n");
      return -1;
    }

    return  1;
  }

  void CaptureGL::flip(bool horizontal, bool vertical) {

    int dx = 0; 
    float* texcoords = NULL;
    float tex_normal[] =     { 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0 } ;
    float tex_vert[] =       { 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0 } ;
    float tex_hori[] =       { 1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0 } ;
    float tex_verthori[] =   { 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 } ;

    if (NULL == pixels || 0 == prog) {
      printf("You're trying to flip the capture device but we're not yet setup.");
      return;
    }

    if (false == horizontal && false == vertical) {
      texcoords = tex_normal;
    }
    else if(false == horizontal && true == vertical) {
      texcoords = tex_vert;
    }
    else if(true == horizontal && false == vertical) {
      texcoords = tex_hori;
    }
    else if (true == horizontal && true == vertical) {
      texcoords = tex_verthori;
    }
    
    if (NULL == texcoords) {
      printf("Error: texcoords == NULL\n");
      return;
    }

    glUseProgram(prog);
    glUniform1fv(u_texcoords, 8, texcoords);
  }

  void CaptureGL::draw() {
    draw(0, 0, win_w, win_h);
  }

  void CaptureGL::draw(int x, int y, int w, int h) {

    glUseProgram(prog);
    glBindVertexArray(vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);

    if(pixel_format == CA_YUV420P) {
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, tex1);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, tex2);
    }

    glUniform4f(u_pos, 
                x * inv_win_w , 
                y * inv_win_h  , 
                w * inv_win_w , 
                h * inv_win_h );

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  void CaptureGL::resize(int w, int h) {
    inv_win_w = 1.0/w;
    inv_win_h = 1.0/h;
    win_w = w;
    win_h = h;
  }

#if defined(_WIN32)
  
  int CaptureGL::createMutex(CaptureMutex& m) {
    InitializeCriticalSection(&m.handle);
    return 0;
  }
  
  int CaptureGL::lockMutex(CaptureMutex& m) {
    EnterCriticalSection(&m.handle);
    return 0;
  }

  int CaptureGL::unlockMutex(CaptureMutex& m) {
    LeaveCriticalSection(&m.handle);
    return 0;
  }

  int CaptureGL::destroyMutex(CaptureMutex& m) {
    DeleteCriticalSection(&m.handle);
    return 0;
  }
  
#elif defined(__linux) or defined(__APPLE__)
  
  int CaptureGL::createMutex(CaptureMutex& m) {
    
    if (0 != pthread_mutex_init(&m.handle, NULL)) {
      printf("Error: failed to create the mutex to sync the pixel data in CaptureGL.\n");
      return -1;
    }
    
    return 0;
  }
  
  int CaptureGL::lockMutex(CaptureMutex& m) {
    
    if (0 != pthread_mutex_lock(&m.handle)) {
      printf("Error: failed to lock the mutex to sync the pixel data in CaptureGL.\n");
      return -1;
    }
    
    return 0;
  }

  int CaptureGL::unlockMutex(CaptureMutex& m) {
    
    if (0 != pthread_mutex_unlock(&m.handle)) {
      printf("Error: failed to unlock the mutex to sync the pixel data in CaptureGL.\n");
      return -1;
    }

    return 0;
  }

  int CaptureGL::destroyMutex(CaptureMutex& m) {
    
    if (0 != pthread_mutex_destroy(&m.handle)) {
      printf("Error: failed to destroy the mutex to sync the pixel data in CaptureGL.\n");
      return -2;
    }

    return 0;
  }
  
#endif
  
  /* checks the compile info, if it didn't compile we return < 0, otherwise 0 */
  static int print_shader_compile_info(GLuint shader) {
 
      GLint status = 0;
      GLint count = 0;
      GLchar* error = NULL;
 
      glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
      if(status) {
         return 0;
      }
 
      error = (GLchar*) malloc(count);
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &count);
      if(count <= 0) {
        free(error);
        error = NULL;
        return 0;
      }
 
      glGetShaderInfoLog(shader, count, NULL, error);
      printf("\nSHADER COMPILE ERROR");
      printf("\n--------------------------------------------------------\n");
      printf("%s", error);
      printf("--------------------------------------------------------\n\n");
 
      free(error);
      error = NULL;
      return -1;
  }

  /* checks + prints program link info. returns 0 when linking didn't result in an error, on link erorr < 0 */
  static int print_program_link_info(GLuint prog) {
    GLint status = 0;
    GLint count = 0;
    GLchar* error = NULL;
    GLsizei nchars = 0;

    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if(status) {
      return 0;
    }

    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &count);
    if(count <= 0) {
      return 0;
    }

    error = (GLchar*)malloc(count);
    glGetProgramInfoLog(prog, count, &nchars, error);
    if (nchars <= 0) {
      free(error);
      error = NULL;
      return -1;
    }

    printf("\nPROGRAM LINK ERROR");
    printf("\n--------------------------------------------------------\n");
    printf("%s", error);
    printf("--------------------------------------------------------\n\n");

    free(error);
    error = NULL;
    return -1;
  }

  /* Returns 0 is extension is supported otherwise -1. */
  static int capturegl_is_extension_supported(std::string name) {

    GLint n=0, i; 
    const char* extension = NULL;

    glGetIntegerv(GL_NUM_EXTENSIONS, &n); 
    for (i=0; i<n; i++) { 
      extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
      if (!strcmp(name.c_str(), extension)) {
        return 0;
      }
    }
    return -1;

    #if 0

    /* GL 2 solution to check extensions!! */

    const char* extension = name.c_str();
    const GLubyte *extensions = NULL;
    const GLubyte *start;
    GLubyte *where, *terminator;

    /* Extension names should not have spaces. */
    where = (GLubyte *) strchr(extension, ' ');
    if (where || *extension == '\0') {
      return -1;
    }

    extensions = glGetStringi(GL_EXTENSIONS, 0);
    if (extensions == NULL) {
      printf("Error: cannot query extensions.\n");
      return -1;
    }

    /* 
       It takes a bit of care to be fool-proof about parsing the
       OpenGL extensions string. Don't be fooled by sub-strings,
       etc. 
    */
    start = extensions;

    for (;;) {
      where = (GLubyte *) strstr((const char *) start, extension);
      if (!where) {
        break;
      }

      terminator = where + strlen(extension);
      if (where == start || *(where - 1) == ' ') {
        if (*terminator == ' ' || *terminator == '\0') {
          return 0;
        }
      }
      start = terminator;
    }
    #endif
    return -1;

  }

} // namespace ca 

#endif // #if defined(VIDEO_CAPTURE_IMPLEMENTATION)


