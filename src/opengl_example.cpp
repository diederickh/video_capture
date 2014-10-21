/*
  
  OpenGL example
  ---------------

  This file contains an example on how to use the capture library 
  when using CA_YUV422 or CA_YUV420P, which are the default (optimal)
  pixel formats for most capture devices. This code was tested on 
  Win 8.1, Mac 10.9 and Arch Linux. 

  The code contains both the CA_YUV422 and CA_YUV420P code-flows 
  and some things can be done to optimize the unpacking of pixel data. 
  Though this will work fine in 99% of the cases.

 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#define ROXLU_IMPLEMENTATION
#include <tinylib.h>

#include <videocapture/Capture.h>
using namespace ca;

// Capture 
void on_signal(int sig);                                /* Signal handler, to correctly shutdown the capture process */
void on_frame(void* bytes, int nbytes, void* user);     /* Gets called whenever new frame data is available */

Capture capture(on_frame, NULL);                        /* The capture object which does all the work */
Frame frame;                                            /* Used to get information about the used pixel format, like bytes, planes, etc.. */
int pix_fmt = CA_NONE;                                  /* The pixel format we want to use; is set below. */
int cap_width = 640;                                    /* We try to query a capability with this height */
int cap_height = 480;                                   /* We try to query a capability with this width */
GLuint prog = 0;                                        /* Shader program */
GLuint vert = 0;                                        /* The vertex shader */
GLuint frag = 0;                                        /* The fragment shader */
GLuint vao = 0;                                         /* Using attribute less renders still needs a vao */
GLuint tex0 = 0;                                        /* Texture for GL_TEXTURE0 unit, used by YUYV422 and YUV420P (y-layer) */
GLuint tex1 = 0;                                        /* Texture for GL_TEXTURE1 unit, used by YUV420P (u-layer) */
GLuint tex2 = 0;                                        /* Texture for GL_TEXTURE2 unit, used by YUV420P (v-layer) */
unsigned char* pixels = NULL;                           /* Pointer to the pixels (non-planar) */
unsigned char* pixels_y = NULL;                         /* Pointer to pixels, y-channel, used with YUV420P */
unsigned char* pixels_u = NULL;                         /* Pointer to pixels, u-channel, used with YUV420P */
unsigned char* pixels_v = NULL;                         /* Pointer to pixels, v-channel, used with YUV420P */
bool must_update_pixels = false;                        /* Is set to true when we need to upload the pixels */

// Shaders
static const char* CAPTURE_VS = ""
  "#version 330\n"
  ""
  "const vec2 pos[] = vec2[4]("
  "  vec2(-1.0,  1.0), "
  "  vec2(-1.0, -1.0), "
  "  vec2( 1.0,  1.0), "
  "  vec2( 1.0, -1.0)  "
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
  "void main() { "
  "  gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);"
  "  v_texcoord = tex[gl_VertexID];"
  "}"
  "";

// Decode YUV422 data (Y0 Cb Y1 Cr)
static const char* CAPTURE_YUYV422_FS = ""
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
  "  float y2 = tc.g; "
  "  float cb = tc.b; "
  "  float y1 = tc.a; "
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
static const char* CAPTURE_YUV420P_FS = ""
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

// GLFW callbacks
void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

int main() {

  signal(SIGINT, on_signal);

  glfwSetErrorCallback(error_callback);
 
  if(!glfwInit()) {
    printf("Error: cannot setup glfw.\n");
    return false;
  }
 
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  GLFWwindow* win = NULL;
  int w = cap_width;
  int h = cap_height;

  win = glfwCreateWindow(w, h, "Capture", NULL, NULL);
  if(!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetFramebufferSizeCallback(win, resize_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwSetCharCallback(win, char_callback);
  glfwSetCursorPosCallback(win, cursor_callback);
  glfwSetMouseButtonCallback(win, button_callback);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);
 
  if (!gladLoadGL()) {
    printf("Cannot load GL.\n");
    exit(1);
  }
 
  // ----------------------------------------------------------------
  // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
  // ----------------------------------------------------------------
  int capability_index = -1;
  int format_index = -1;

  capture.listCapabilities(0);
 
#if defined(__APPLE__) || defined(__linux)
  pix_fmt = CA_YUYV422; 
#elif defined(_WIN32)
  pix_fmt = CA_YUV420P;
#endif

  // Find the capability 
  capability_index = capture.findCapability(0, cap_width, cap_height, pix_fmt);
  if(capability_index < 0) {
    capture.listCapabilities(0);
    printf("Error: cannot find a capability for 640x480.\n");
    ::exit(EXIT_FAILURE);
  }
  
  // Check what output formats are supports (not all OSes support these)
  //capture.listOutputFormats();

  // Create the settings param 
  Settings settings;
  settings.device = 0;
  settings.capability = capability_index;
  settings.format = format_index;

  // Open the capture device
  if(capture.open(settings) < 0) {
    printf("Error: cannot open the device.\n");
    ::exit(EXIT_FAILURE);
  }

  // Create the pixel buffer, we use Frame to get info about the size of the data for the current pixel format
  if(frame.set(cap_width, cap_height, pix_fmt) < 0) {
    printf("Error: cannot get frame information for the current pixel format.\n");
    capture.close();
    ::exit(EXIT_FAILURE);
  }

  // Create the GL objects we use to render 
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  vert = rx_create_shader(GL_VERTEX_SHADER, CAPTURE_VS);

  // Get the correct fragment shader
  if(pix_fmt == CA_YUYV422) {
    frag = rx_create_shader(GL_FRAGMENT_SHADER, CAPTURE_YUYV422_FS);
  }
  else if(pix_fmt == CA_YUV420P) {
    frag = rx_create_shader(GL_FRAGMENT_SHADER, CAPTURE_YUV420P_FS);
  }

  prog = rx_create_program(vert, frag);
  glLinkProgram(prog);
  rx_print_shader_link_info(prog);
  glUseProgram(prog);

  if(pix_fmt == CA_YUYV422) {

    pixels = new unsigned char[frame.nbytes[0]];

    // CA_YUYV422 texture
    glUniform1i(glGetUniformLocation(prog, "u_tex"), 0);
  
    glGenTextures(1, &tex0);
    glBindTexture(GL_TEXTURE_2D, tex0);

#if defined(__APPLE__)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.width[0], frame.height[0], 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
#elif defined(__linux)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.width[0], frame.height[0], 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);
#endif

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }
  else if(pix_fmt == CA_YUV420P) {

    pixels_y = new unsigned char[frame.nbytes[0]];
    pixels_u = new unsigned char[frame.nbytes[1]];
    pixels_v = new unsigned char[frame.nbytes[2]];

    // CA_YUV420P textures
    glUniform1i(glGetUniformLocation(prog, "y_tex"), 0);
    glUniform1i(glGetUniformLocation(prog, "u_tex"), 1);
    glUniform1i(glGetUniformLocation(prog, "v_tex"), 2);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // y-channel
    glGenTextures(1, &tex0); 
    glBindTexture(GL_TEXTURE_2D, tex0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, frame.width[0], frame.height[0], 0, GL_RED, GL_UNSIGNED_BYTE, pixels_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // u-channel
    glGenTextures(1, &tex1); 
    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, frame.width[1], frame.height[1], 0, GL_RED, GL_UNSIGNED_BYTE, pixels_u);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // v-channel
    glGenTextures(1, &tex2); 
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, frame.width[2], frame.height[2], 0, GL_RED, GL_UNSIGNED_BYTE, pixels_v);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  
  // Start captureing
  if(capture.start() < 0) {
    printf("Error: cannot start capture.\n");
    capture.close();
    ::exit(EXIT_FAILURE);
  }

  while(!glfwWindowShouldClose(win)) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    capture.update();

    if(must_update_pixels) {

      // Update YUYV422
      if(pix_fmt == CA_YUYV422) {

        glBindTexture(GL_TEXTURE_2D, tex0);

#if defined(__APPLE__)
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[0], frame.height[0], GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
#elif defined(__linux)
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[0], frame.height[0], GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);
#endif
      }
      // Update YUV420P
      else if(pix_fmt == CA_YUV420P) {
        glBindTexture(GL_TEXTURE_2D, tex0);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[0], frame.height[0], GL_RED, GL_UNSIGNED_BYTE, pixels_y);

        glBindTexture(GL_TEXTURE_2D, tex1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[1], frame.height[1], GL_RED, GL_UNSIGNED_BYTE, pixels_u);

        glBindTexture(GL_TEXTURE_2D, tex2);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[2], frame.height[2], GL_RED, GL_UNSIGNED_BYTE, pixels_v);
      }
      must_update_pixels = false;
    }
    
    if(pix_fmt == CA_YUYV422) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex0);
    }
    else if(pix_fmt == CA_YUV420P) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex0);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, tex1);

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, tex2);
    }

    glUseProgram(prog);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  if(capture.stop() < 0) {
    printf("Error: cannot stop capture.\n");
    capture.close();
    ::exit(EXIT_FAILURE);
  }

  if(capture.close() < 0) {
    printf("Error: cannot close the device.\n");
    ::exit(EXIT_FAILURE);
  }
  glfwTerminate();
 
  return EXIT_SUCCESS;
}
 
void char_callback(GLFWwindow* win, unsigned int key) {
}
 
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
  
  if(action != GLFW_PRESS) {
    return;
  }

  if(action == GLFW_PRESS) { 
  }
  else if(action == GLFW_RELEASE) {
  }
 
  switch(key) {
    case GLFW_KEY_SPACE: {
      break;
    }
    case GLFW_KEY_ESCAPE: {
      glfwSetWindowShouldClose(win, GL_TRUE);
      break;
    }
  };
}
 
void resize_callback(GLFWwindow* window, int width, int height) {
}

void cursor_callback(GLFWwindow* win, double x, double y) {
}

void button_callback(GLFWwindow* win, int bt, int action, int mods) {

  double x,y;

  if(action == GLFW_PRESS || action == GLFW_REPEAT) { 
    glfwGetCursorPos(win, &x, &y);
  }

  if(action == GLFW_PRESS) {
  }
  else if(action == GLFW_RELEASE) {
  }
}

void error_callback(int err, const char* desc) {
  printf("GLFW error: %s (%d)\n", desc, err);
}

void on_frame(void* bytes, int nbytes, void* user) {
  // This function might be called from a different thread so we copy it over and change a flag.
  // In the event/draw loop we will update the GL textures from the main thread.
  if(pix_fmt == CA_YUYV422) {
    memcpy(pixels, (char*)bytes, nbytes);
  }
  else if(pix_fmt == CA_YUV420P) {
    memcpy(pixels_y, ((char*)bytes) + frame.offset[0], frame.nbytes[0]);
    memcpy(pixels_u, ((char*)bytes) + frame.offset[1], frame.nbytes[1]);
    memcpy(pixels_v, ((char*)bytes) + frame.offset[2], frame.nbytes[2]);
  }
  must_update_pixels = true;
}

void on_signal(int sig) {
  capture.stop();
  capture.close();
  ::exit(EXIT_FAILURE);
}
