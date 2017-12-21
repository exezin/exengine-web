#include "window.h"
#include "math.h"
#include <stdio.h>

ex_window_t display;
uint8_t ex_keys_down[GLFW_KEY_LAST];
uint8_t ex_buttons_down[GLFW_KEY_LAST];

int ex_window_init(uint32_t width, uint32_t height, const char *title)
{
  // init glfw stuffs
  glfwInit();
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  
  // create glfw window
  display.window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (display.window == NULL) {
    printf("Failed to initialize window\n");
    glfwTerminate();
    return 0;
  }

  // set callbacks
  glfwSetKeyCallback(display.window, ex_key_callback);
  glfwSetCursorPosCallback(display.window, ex_mouse_callback);
  glfwSetFramebufferSizeCallback(display.window, ex_resize_callback);
  glfwSetMouseButtonCallback(display.window, ex_button_callback);
  glfwSetScrollCallback(display.window, ex_scroll_callback);
  glfwSetCharCallback(display.window, ex_char_callback);

  // set context
  glfwMakeContextCurrent(display.window);

  // init glew
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    printf("Error initializing glew\n%s\n", glewGetErrorString(err));
    return 0;
  }
  
  // set viewport etc
  glViewport(0, 0, width, height);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glCullFace(GL_BACK);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LEQUAL);

  glfwSetInputMode(display.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSwapInterval(0);

  // set html5 key callback
#ifdef __EMSCRIPTEN__
  emscripten_set_keypress_callback(0, 0, 1, ex_ehandle_keys);
  emscripten_set_keydown_callback(0, 0, 1, ex_ehandle_keys);
  emscripten_set_keyup_callback(0, 0, 1, ex_ehandle_keys);
  emscripten_set_mousemove_callback(0, 0, 1, ex_ehandle_mouse);
  emscripten_set_click_callback(0, 0, 1, ex_ehandle_mouse);
  emscripten_set_mousedown_callback(0, 0, 1, ex_ehandle_mouse);
  emscripten_set_mouseup_callback(0, 0, 1, ex_ehandle_mouse);
  emscripten_set_dblclick_callback(0, 0, 1, ex_ehandle_mouse);
#endif

  return 1;
}

void ex_window_begin()
{
  glfwPollEvents();
}

void ex_window_end()
{
  glfwSwapBuffers(display.window);
}

void ex_window_destroy()
{
  glfwTerminate();
}

void ex_resize_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}

void ex_mouse_callback(GLFWwindow* window, double x, double y)
{
  if (glfwGetInputMode(display.window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
    return;

  display.mouse_x = x;
  display.mouse_y = y;
}

void ex_key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
  if (glfwGetInputMode(display.window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
    return;

  if (action == GLFW_PRESS)
    ex_keys_down[key] = 1;
  if (action == GLFW_RELEASE)
    ex_keys_down[key] = 0;
}

void ex_button_callback(GLFWwindow *window, int button, int action, int mods)
{
  if (glfwGetInputMode(display.window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
    return;

  if (action == GLFW_PRESS)
    ex_buttons_down[button] = 1;
  if (action == GLFW_RELEASE)
    ex_buttons_down[button] = 0;
}

void ex_char_callback(GLFWwindow *window, unsigned int c)
{

}

void ex_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{

}

#ifdef __EMSCRIPTEN__
EM_BOOL ex_ehandle_keys(int type, const EmscriptenKeyboardEvent *e, void *user_data)
{
  // prevent scrolling with arrow keys and stuff when focused
  if (glfwGetInputMode(display.window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    return 1;

  return 0;
}

EM_BOOL ex_ehandle_mouse(int type, const EmscriptenMouseEvent *e, void *user_data)
{
  // prevent scrolling and stuff when focused
  if (glfwGetInputMode(display.window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
    return 1;

// DONT BLOODY WORK DOES IT
// #ifdef __EMSCRIPTEN__
  // if (type == EMSCRIPTEN_EVENT_MOUSEMOVE && 
    // e->screenX != 0 && e->screenY != 0 && e->clientX != 0 && e->clientY != 0 && e->canvasX != 0 && e->canvasY != 0 && e->targetX != 0 && e->targetY != 0) {
    // double x = e->movementX;
    // double y = e->movementY;
    // float max = 100.0f;
    
    // if (x > max || x < -max)
      // return 0;
    // if (y > max || y < -max)
      // return 0;

    // printf("%f %f\n", x, y);
  // }
// #endif

  return 0;
}
#endif