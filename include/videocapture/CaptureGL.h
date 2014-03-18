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
  
      #define VIDEOCAPTURE_IMPLEMENTATION
      #include <videocapture/CaptureGL.h>  

  
      // ...

      
      ca::CaptureGL capture;
      
      if(capture.open(0, 640, 480) < 0) {
        printf("Error: Can't open the capture device.\n");
        ::exit(EXIT_FAILURE);
      }
    
      while(rendering) {
        capture.update();
        capture.draw(10, 10, 320, 240);
      }
    
      capture.stop();
      capture.close(); 
  ````      

*/
#ifndef VIDEO_CAPTURE_GL_H
#define VIDEO_CAPTURE_GL_H

#include <videocapture/Types.h>
#include <videocapture/Capture.h>

static const char* CAPTURE_GL_VS = ""
  "#version 330\n"
  "uniform vec4 u_pos; " 
  ""
  "const vec2 pos[] = vec2[4]("
  "  vec2(0.0,  1.0), "
  "  vec2(0.0, 0.0), "
  "  vec2( 1.0,  1.0), "
  "  vec2( 1.0, 0.0)  "
  ");"
  ""
  " const vec2[] tex = vec2[4]( "
  "   vec2(0.0, 0.0), "
  "   vec2(0.0, 1.0), "
  "   vec2(1.0, 0.0), "
  "   vec2(1.0, 1.0) "
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
  "  v_texcoord = tex[gl_VertexID];"
  "}"
  "";

// Decode YUV422 data (Y0 Cb Y1 Cr)
static const char* CAPTURE_GL_YUYV422_FS = ""
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

// Decode YUV420P (3 planes)
static const char* CAPTURE_GL_YUV420P_FS = ""
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

  void capturegl_on_frame(void* pixels, int nbytes, void* user);                      /* Is called whenever a frame is received from the capture device */
  GLuint capturegl_create_shader(GLenum type, const char* source);                    /* Creates a shader for the given type. */
  
  class CaptureGL {
    
  public:
    CaptureGL();
    ~CaptureGL();
    int open(int device, int width, int height);                                       /* Setup the capturer for the given device and dimensions. We will try to find the most optimal pixel format format. This function will return -1 on error, else the same as the given divice */
    int close();                                                                       /* Close the capture device */
    int start();                                                                       /* Start capturing */ 
    int stop();                                                                        /* Stop capturing */
    int update();                                                                      /* Call this regularly. Returns 1 when we update pixels, -1 on error and else 0. */
    void resize(int w, int h);                                                         /* Call w/h when the viewport resizes */
    void draw();                                                                       /* Draw, filling the the viewport */
    void draw(int x, int y, int w, int h);                                             /* Draw the current frame at x,y and w/h */

  private:
    int setupGraphics();                                                               /* Creates the opengl objects */
    int setupShaders();                                                                /* Creates shaders for the pixel format */
    int setupTextures();                                                               /* Creates the textures that back the current pixel format */
    void updateYUYV422();                                                              /* Updates the YUYV422 pixel data */
    void updateYUV420P();

  public:
    int fmt;                                                                           /* The pixel format we use. Used to setup graphics state */
    int width;                                                                         /* The width of the frames we capture */
    int height;                                                                        /* The height of the frames we capture */
    float inv_win_w;                                                                   /* Inverse window/viewport width, used to calculate the correct position/offset values for the vertex shader . */
    float inv_win_h;                                                                   /* Inverse window/viewport height, used to calculate the correct position/offset values for the vertex shader . */
    int win_w;
    int win_h;
    Frame frame;                                                                       /* The `Frame` object is used to get information about width/height/bytes used for the current pixel format */ 
    Capture cap;                                                                       /* We use the Capture class for all capture related things */
    GLuint frag;                                                                       /* The fragment shader that does the conversion */
    GLuint vert;                                                                       /* Simple vertex shader using attribute less rendering */
    GLuint prog;                                                                       /* The shader program */  
    GLuint vao;                                                                        /* We need a VAO to render attribute-less */
    GLint u_pos;                                                                       /* Points to the u_pos in the vertex shader; used to scale and offset the vertex position */
    GLint u_tex0;                                                                      /* Points to the first texture element in the frag shader. */
    GLint u_tex1;                                                                      /* Points to the second plane (e.g. when format is YUV420P) */
    GLint u_tex2;                                                                      /* Points to the thirds plane (e.g. when format is YUV420P) */
    GLuint tex0;                                                                       /* Texture that will be filled with the pixel data from the webcam */
    GLuint tex1;                                                                       /* Texture that will be filled with the pixel data from the webcam (e.g. the U plane when using YUV420P). */
    GLuint tex2;                                                                       /* Texture that will be filled with the pixel data from the webcam (e.g. the U plane when using YUV420P). */
    unsigned char* pixels;                                                             /* When we use the YUYV422/YUV420 this will hold all the data */
    bool needs_update;                                                                 /* Is set to true when we receive a new frame */
  };

  // Update the YUYV422 Pixels
  inline void CaptureGL::updateYUYV422() {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[0], frame.height[0], GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
  }

  // Update the YUV420P Pixels
  inline void CaptureGL::updateYUV420P() {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glBindTexture(GL_TEXTURE_2D, tex0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[0], frame.height[0], GL_RED, GL_UNSIGNED_BYTE, pixels + frame.offset[0]);

    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[1], frame.height[1], GL_RED, GL_UNSIGNED_BYTE, pixels + frame.offset[1]);

    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[2], frame.height[2], GL_RED, GL_UNSIGNED_BYTE, pixels + frame.offset[2]);
  }


#if defined(VIDEO_CAPTURE_IMPLEMENTATION)
  /* -------------------------------------- */

  void capturegl_on_frame(void* pixels, int nbytes, void* user) {

    if(nbytes == 0 || pixels == NULL) {
      return;
    }

    CaptureGL* gl = static_cast<CaptureGL*>(user);

    if(gl->fmt == CA_YUYV422 || gl->fmt == CA_YUV420P) {
      memcpy((char*)gl->pixels, (char*)pixels, nbytes);
      gl->needs_update = true;
    }
    else {
      printf("Error: pixels not handled in capturegl_on_frame().\n");
    }
  }

  GLuint capturegl_create_shader(GLenum type, const char* source) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &source, NULL);
    glCompileShader(s);
    return s;
  }

  /* -------------------------------------- */

  CaptureGL::CaptureGL()
    :cap(capturegl_on_frame, this)
    ,fmt(CA_NONE)
    ,width(0)
    ,height(0)
    ,inv_win_w(0)
    ,inv_win_h(0)
    ,win_w(0)
    ,win_h(0)
    ,frag(0)
    ,vert(0)
    ,prog(0)
    ,vao(0)
    ,u_pos(0)
    ,u_tex0(0)
    ,u_tex1(0)
    ,u_tex2(0)
    ,tex0(0)
    ,tex1(0)
    ,tex2(0)
    ,pixels(NULL)
    ,needs_update(false)
    {
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
    u_tex0 = 0;
    u_tex1 = 0;
    u_tex2 = 0;
    
    // @todo - free GL 
  }

  int CaptureGL::open(int device, int w, int h) {

    width = w;
    height = h;

    Settings settings;
    settings.device = device;

    //cap.listCapabilities(device);

    // find the best matching capability
    int caps[] = { CA_YUYV422, CA_YUV420P } ;
    int num_caps = 2;
    for(int i = 0; i < num_caps; ++i) {
      settings.capability = cap.findCapability(device, width, height, caps[i]);
      if(settings.capability >= 0) {
        fmt = caps[i];
        break;
      }
    }
    if(settings.capability < 0) {
      printf("Error: cannot find a usable capability.\n");
      return -1;
    }

    // find the best output format
    std::vector<Format> formats = cap.getOutputFormats();
    for(size_t i = 0; i < formats.size(); ++i) {
      Format& f = formats[i];
      for(int j = 0; j < num_caps; ++j) {
        if(f.format == caps[j]) {
          settings.format = f.index;
          break;
        }
      }
    }

    // for now we set the format to the capability.
    if(settings.format == -1) {
      settings.format = settings.capability;
    }

    if(cap.open(settings) < 0) {
      printf("Error: cannot open the capture device.\n");
      return -2;
    }

    if(setupGraphics() < 0) {
      printf("Error: cannot setup the GL graphics.\n");
      return -3;
    }

    // set the scale factor.
    GLint vp[4]; 
    glGetIntegerv(GL_VIEWPORT, vp);
    resize(vp[2], vp[3]);

    return device;
  }

  int CaptureGL::close() {
    return cap.close();
  }

  int CaptureGL::start() {
    return cap.start();
  }

  int CaptureGL::update() {
    
    if(needs_update) {

      if(fmt == CA_YUV420P) {
        updateYUV420P();
      }
      else if(fmt == CA_YUYV422) {
        updateYUYV422();
      }

      needs_update = false;
    }

    cap.update();

    return needs_update;
  }

  int CaptureGL::stop() {
    return cap.stop();
  }

  int CaptureGL::setupGraphics() {

    glGenVertexArrays(1, &vao);    
        
    if(setupShaders() < 0) {
      return -1;
    }

    if(setupTextures() < 0) {
      return -2;
    }

    return 1;
  }

  int CaptureGL::setupShaders() {
    
    vert = capturegl_create_shader(GL_VERTEX_SHADER, CAPTURE_GL_VS);

    // select the correct fragment shader
    if(fmt == CA_YUV420P) {
      frag = capturegl_create_shader(GL_FRAGMENT_SHADER, CAPTURE_GL_YUV420P_FS);
    }
    else if(fmt == CA_YUYV422) {
      frag = capturegl_create_shader(GL_FRAGMENT_SHADER, CAPTURE_GL_YUYV422_FS);
    }
    else {
      printf("Error: no shader yet for the current pixel format. \n");
      return -1;
    }

    prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    glUseProgram(prog);

    if(fmt == CA_YUV420P) {
      u_tex0 = glGetUniformLocation(prog, "y_tex");
      u_tex1 = glGetUniformLocation(prog, "u_tex");
      u_tex2 = glGetUniformLocation(prog, "v_tex");
      glUniform1i(u_tex0, 0);
      glUniform1i(u_tex1, 1);
      glUniform1i(u_tex2, 2);
    }
    else if(fmt == CA_YUYV422) {
      u_tex0 = glGetUniformLocation(prog, "u_tex");
      glUniform1i(u_tex0, 0);
    }
    
    u_pos = glGetUniformLocation(prog, "u_pos");

    glUseProgram(0);
    return 1;
  }

  int CaptureGL::setupTextures() {

    if(frame.set(width, height, fmt) < 0) {
      printf("Error: cannot set the frame object.\n");
      return -1;
    }
    
    // create textures to the back the pixel format.
    if(fmt == CA_YUV420P) {
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
    else if(fmt == CA_YUYV422) {

      pixels = new unsigned char[frame.nbytes[0]];

      glGenTextures(1, &tex0);
      glBindTexture(GL_TEXTURE_2D, tex0);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.width[0], frame.height[0], 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
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

  void CaptureGL::draw() {
    draw(0, 0, win_w, win_h);
  }

  void CaptureGL::draw(int x, int y, int w, int h) {

    glUseProgram(prog);
    glBindVertexArray(vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);

    if(fmt == CA_YUV420P) {
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

#endif // #if defined(VIDEO_CAPTURE_IMPLEMENTATION)

} // namespace ca

#endif
