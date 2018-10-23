#include "engine.h"
#include "camera.h"
#include "texture.h"
#include "pointlight.h"
#include "dirlight.h"
#include "scene.h"
#include "exe_list.h"
#include "iqm.h"
#include "skybox.h"
#include "entity.h"
#include "framebuffer.h"
#include "defaults.h"
#include "cache.h"

#include <emscripten/emscripten.h>

// renderer feature toggles
int ex_enable_ssao = 1;

// user defined function callback pointers
void (*ex_init_ptr)(void);
void (*ex_update_ptr)(double);
void (*ex_draw_ptr)(void);
void (*ex_exit_ptr)(void);
// non-essential user callbacks
void (*ex_keypressed_ptr)(int, int, int, int);
void (*ex_mousepressed_ptr)(int, int, int);
void (*ex_keyinput_ptr)(unsigned int);
void (*ex_mousescroll_ptr)(double, double);
void (*ex_resize_ptr)(int, int);

conf_t conf;
// delta time vars
const double phys_delta_time = 1.0 / 60.0;
const double slowest_frame = 1.0 / 15.0;
double delta_time, accumulator = 0.0;
double last_ex_frame_time = 0;

void exengine_loop();

void exengine(char **argv, uint8_t flags)
{
  /* -- INIT ENGINE -- */

  // init engine file data cache
  ex_cache_init();

  // init subsystems
  if (flags & EX_ENGINE_SOUND)
    ex_sound_init();
 
  // load user defined config
  conf_load(&conf, "data/conf.cfg");

  // load config vars
  uint32_t width = 0, height = 0;
  width = conf_get_int(&conf, "window_width");
  height = conf_get_int(&conf, "window_height");
  
  // init the window and gl
  if (!ex_window_init(width, height, "exengine-testing")) {
    ex_exit_ptr();
    return;
  }

  const char ES_VERSION_2_0[] = "OpenGL ES 2.0 (WebGL 1.0)";
  const char ES_VERSION_3_0[] = "OpenGL ES 3.0 (WebGL 2.0)";
  const char *gl_version = glGetString(GL_VERSION);
  printf("Using %s\n", gl_version);
  /*if (strncmp(version, ES_VERSION_2_0, sizeof(ES_VERSION_2_0)) == 0) {
    printf("WEBGL1!\n");
  } else if (strncmp(version, ES_VERSION_3_0, sizeof(ES_VERSION_3_0)) == 0) {
    printf("WEBGL2!\n");
  }*/

  // init rendering modules
  ex_defaults_textures();
  ex_framebuffer_init();
  
  // user init callback
  ex_init_ptr();
  /* ----------------- */

  /* -- UPDATE ENGINE -- */
  last_ex_frame_time = glfwGetTime();
  emscripten_set_main_loop(exengine_loop, 0, 1);
  /* ------------------- */        


  // -- CLEAN UP -- */
  conf_free(&conf);
  ex_window_destroy();
  ex_cache_flush();
  ex_framebuffer_cleanup();
  if (flags & EX_ENGINE_SOUND)
    ex_sound_exit();

  // user exit callback
  ex_exit_ptr();
  // -------------- */
}

void exengine_loop()
{
  // handle window events
  ex_window_begin();

  // calculate delta time
  double current_ex_frame_time = (double)glfwGetTime();
  delta_time = current_ex_frame_time - last_ex_frame_time;
  last_ex_frame_time = current_ex_frame_time;

  // prevent spiral of death
  if (delta_time > slowest_frame)
    delta_time = slowest_frame;

  // update at a constant rate to keep physics in check
  accumulator += delta_time;
  while (accumulator >= phys_delta_time) {
    glfwPollEvents();

    // user update callback
    ex_update_ptr(phys_delta_time);

    accumulator -= phys_delta_time;
  }

  
  // user draw callback
  ex_draw_ptr();

  // swap buffers render gui etc
  ex_window_end();
  glfwSwapBuffers(display.window);
}