#include <stdio.h>
#include <stdlib.h>

#if defined(__linux) || defined(_WIN32)
#  include <GLXW/glxw.h>
#endif
 
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#define ROXLU_USE_PNG
#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#define ROXLU_IMPLEMENTATION
#include <tinylib.h>

#include <videocapture/Capture.h>
using namespace ca;

// Capture 
void on_frame(void* bytes, int nbytes, void* user);
Capture capture(on_frame, NULL);

int cap_width = 640;
int cap_height = 480;
GLuint prog = 0;
GLuint vert = 0;
GLuint frag = 0;
GLuint vao = 0;
GLuint vbo = 0;
GLuint tex0 = 0;  /* Texture for GL_TEXTURE0 unit */
unsigned char* pixels = NULL; /* Pointer to the pixels */
bool must_update_pixels = false; /* Is set to true when we need to upload the pixels */

// Shaders
static const char* CAPTURE_VS = ""
  "#version 330\n"
  "uniform mat4 u_pm;"
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
  "out vec2 v_tex; "
  ""
  "void main() { "
  "  gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);"
  "  v_tex = tex[gl_VertexID];"
  "}"
  "";

static const char* CAPTURE_YUYV422_FS = ""
  "#version 330\n"
  ""
  "uniform sampler2D u_tex;"
  "layout( location = 0 ) out vec4 fragcolor; "
  "in vec2 v_tex;"
  ""
  "const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);"
  "const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);"
  "const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);"
  "const vec3 offset = vec3(-0.0625, -0.5, -0.5);"
  ""
  "void main() {"
  ""
  "  int width = textureSize(u_tex, 0).x * 2;"
  "  float tex_x = v_tex.x; "
  "  int pixel = int(floor(width * tex_x)) % 2; "
  "  vec4 tc =  texture( u_tex, v_tex ).rgba;"
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

// GLFW callbacks
void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

int main() {

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
 
#if defined(__linux) || defined(_WIN32)
  if(glxwInit() != 0) {
    printf("Error: cannot initialize glxw.\n");
    ::exit(EXIT_FAILURE);
  }
#endif
 
  // ----------------------------------------------------------------
  // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
  // ----------------------------------------------------------------
  int pix_fmt = CA_YUYV422; 
  int capability_index = -1;
  int format_index = -1;
  cap_width = 640;
  cap_height = 480;
  
#if defined(__APPLE__)

  // Find the capability 
  capability_index = capture.findCapability(0, cap_width, cap_height, pix_fmt);
  if(capability_index < 0) {
    capture.listCapabilities(0);
    printf("Error: cannot find a capability for 640x480.\n");
    ::exit(EXIT_FAILURE);
  }

  capture.listOutputFormats();

#endif

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
  Frame frame;
  if(frame.set(cap_width, cap_height, pix_fmt) < 0) {
    printf("Error: cannot get frame information for the current pixel format.\n");
    ::exit(EXIT_FAILURE);
  }
  printf("We need: %d bytes for the texture.\n", frame.nbytes[0]);
  pixels = new unsigned char[frame.nbytes[0]];

  // Create the GL objects we use to render 
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  vert = rx_create_shader(GL_VERTEX_SHADER, CAPTURE_VS);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, CAPTURE_YUYV422_FS);
  prog = rx_create_program(vert, frag);
  glLinkProgram(prog);
  rx_print_shader_link_info(prog);

  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "u_tex"), 0);
  
  glGenTextures(1, &tex0);
  glBindTexture(GL_TEXTURE_2D, tex0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.width[0], frame.height[0], 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  
  // Start captureing
  if(capture.start() < 0) {
    printf("Error: cannot start capture.\n");
    ::exit(EXIT_FAILURE);
  }

  while(!glfwWindowShouldClose(win)) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);
    
    if(must_update_pixels) {
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width[0], frame.height[0], GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
      must_update_pixels = false;
    }
    
    glUseProgram(prog);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  if(capture.stop() < 0) {
    printf("Error: cannot stop capture.\n");
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
  memcpy(pixels, (char*)bytes, nbytes);
  must_update_pixels = true;
}
