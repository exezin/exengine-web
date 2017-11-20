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

GLuint shader;
ex_fps_camera_t *camera;
ex_scene_t *scene;
ex_model_t *dude;

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

  // start game loop
  emscripten_set_main_loop(do_frame, 0, 0);
}

void do_frame()
{
  // update
  ex_fps_camera_update(camera, shader);

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

  // update and render scene
  // *FIX DELTA TIME HERE*
  ex_scene_update(scene, 0.05f);
  ex_scene_draw(scene);

  glfwSwapBuffers(display.window);
}
