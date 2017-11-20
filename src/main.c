#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <emscripten/emscripten.h>

#include "shader.h"
#include "window.h"
#include "camera.h"
#include "texture.h"
#include "scene.h"
#include "iqm.h"

// scene stuff
GLuint shader;
ex_fps_camera_t *camera;
ex_scene_t *scene;
ex_model_t *dude;

// timestep stuff
const double phys_delta_time = 1.0 / 60.0;
const double slowest_frame = 1.0 / 15.0;
double delta_time, last_frame_time, accumulator = 0.0;

void do_frame();

int main()
{
  ex_window_init(640, 480, "emscripten");

  shader = ex_shader_compile("data/shader.vs", "data/shader.fs");

  // setup a fps camera
  camera = ex_fps_camera_new(-1.5f, 1.0f, 0.0f, 0.1f, 75.0f);

  // setup the scene
  scene = ex_scene_new(shader);
  scene->fps_camera = camera;

  // load a human model
  dude = ex_iqm_load_model(scene, "data/dude.iqm");
  list_add(scene->model_list, dude);

  // set to animating
  ex_model_set_anim(dude, 0);

  last_frame_time = glfwGetTime();

  // start game loop
  emscripten_set_main_loop(do_frame, 0, 0);
}

void do_frame()
{
  // delta time shiz
  double current_frame_time = glfwGetTime();
  delta_time = current_frame_time - last_frame_time;
  last_frame_time = current_frame_time;

  accumulator += delta_time;
  while (accumulator >= phys_delta_time) {
    // move the camera with wasd
    vec3 speed, side;
    if (ex_keys_down[GLFW_KEY_W]) {
      vec3_scale(speed, camera->front, 0.2f);
      vec3_add(camera->position, camera->position, speed);
    }
    if (ex_keys_down[GLFW_KEY_S]) {
      vec3_scale(speed, camera->front, 0.2f);
      vec3_sub(camera->position, camera->position, speed);
    }
    if (ex_keys_down[GLFW_KEY_A]) {
      vec3_mul_cross(side, camera->front, camera->up);
      vec3_norm(side, side);
      vec3_scale(side, side, 0.2f);
      vec3_sub(camera->position, camera->position, side);
    }
    if (ex_keys_down[GLFW_KEY_D]) {
      vec3_mul_cross(side, camera->front, camera->up);
      vec3_norm(side, side);
      vec3_scale(side, side, 0.2f);
      vec3_add(camera->position, camera->position, side);
    }
    
    ex_scene_update(scene, phys_delta_time);

    accumulator -= phys_delta_time;
  }

  // render scene
  ex_scene_draw(scene);

  glfwSwapBuffers(display.window);
}
