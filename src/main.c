#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "shader.h"
#include "window.h"
#include "camera.h"
#include "texture.h"
#include "scene.h"
#include "iqm.h"
#include "text.h"
#include "sound.h"
#include "entity.h"

// scene stuff
GLuint shader;
ex_fps_camera_t *camera;
ex_scene_t *scene;
ex_model_t *dude, *level;
ex_font_t *raleway;
ex_source_t *sound;
ex_entity_t *player;
ex_point_light_t *plight;

// timestep stuff
const double phys_delta_time = 1.0 / 60.0;
const double slowest_frame = 1.0 / 15.0;
double delta_time, last_frame_time, accumulator = 0.0;

// physics stuff
float move_speed = 200.0f;

void do_frame();
void at_exit();

int main()
{
  ex_window_init(640, 480, "exengine");

  shader = ex_shader_compile("data/shaders/shader.vs", "data/shaders/shader.fs");

  // setup a fps camera
  camera = ex_fps_camera_new(-1.5f, 1.0f, 0.0f, 0.1, 90.0);

  // setup the scene
  scene = ex_scene_new(shader);
  scene->fps_camera = camera;

  // load a human model
  dude = ex_iqm_load_model(scene, "data/dude.iqm", 0);
  list_add(scene->model_list, dude);

  // load a level with collision
  level = ex_iqm_load_model(scene, "data/level.iqm", 1);
  list_add(scene->model_list, level);

  // set to animating
  ex_model_set_anim(dude, 0);

  last_frame_time = glfwGetTime();

  // load a font
  raleway = ex_text_load_font("data/fonts/raleway.ttf", 48);

  // load a sound
  sound = ex_sound_load_source("data/sound/test.ogg", EX_SOUND_OGG, 1);
  alSourcePlay(sound->id);
  ex_sound_master_volume(0.0f);

  // setup the player entity
  player = ex_entity_new(scene, (vec3){0.5f, 1.0f, 0.5f});
  player->position[1] = 1.1f;
  player->position[0] = 1.1f; 
  player->position[2] = 5.0f;

  // add some lighting to the scene
  ex_point_light_t *light = ex_point_light_new(player->position, (vec3){0.8f, 0.8f, 0.8f}, 1);
  light->position[1] = 5.0f;
  ex_scene_add_pointlight(scene, light);
  ex_point_light_t *light2 = ex_point_light_new((vec3){-2.5f, 5.0f, -20.0f}, (vec3){1.0f, 1.0f, 1.5f}, 1);
  ex_scene_add_pointlight(scene, light2);
  plight = ex_point_light_new((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.3f, 0.3f, 0.3f}, 0);
  plight->is_shadow = 0;
  ex_scene_add_pointlight(scene, plight);

  // start game loop
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(do_frame, 0, 0);
#else
  while (!glfwWindowShouldClose(display.window)) {
    do_frame();
  }

  at_exit();
  ex_scene_destroy(scene);
#endif
}

void do_frame()
{
  // delta time shiz

  double current_frame_time = (double)glfwGetTime();
  delta_time = current_frame_time - last_frame_time;
  last_frame_time = current_frame_time;

  accumulator += delta_time;
  while (accumulator >= phys_delta_time) {
    ex_window_begin();
    
    // run update entity doing physics collision etc
    ex_entity_update(player, phys_delta_time);

    // set camera to entity position
    memcpy(camera->position, player->position, sizeof(vec3));
    camera->position[1] += player->radius[1];

    memcpy(plight->position, player->position, sizeof(vec3));
    
    // some basics fps movement
    vec3 temp;
    vec3_scale(temp, player->velocity, 25.0f * phys_delta_time);
    temp[1] = 0.0f;

    // do friction and slow air movement
    if (player->grounded == 1) {
      vec3_sub(player->velocity, player->velocity, temp);
      move_speed = 200.0f;
    } else {
      move_speed = 20.0f;
    }
    
    // do gravity
    player->velocity[1] -= (100.0f * phys_delta_time);
    if (player->velocity[1] <= 0.0f && player->grounded)
      player->velocity[1] = 0.0f;

    // wasd movement
    vec3 speed, side;
    if (ex_keys_down[GLFW_KEY_W]) {
      vec3_norm(speed, (vec3){camera->front[0], 0.0f, camera->front[2]});
      vec3_scale(speed, speed, move_speed * phys_delta_time);
      speed[1] = 0.0f;
      vec3_add(player->velocity, player->velocity, speed);
    }
    if (ex_keys_down[GLFW_KEY_S]) {
      vec3_norm(speed, (vec3){camera->front[0], 0.0f, camera->front[2]});
      vec3_scale(speed, speed, move_speed * phys_delta_time);
      speed[1] = 0.0f;
      vec3_sub(player->velocity, player->velocity, speed);
    }
    if (ex_keys_down[GLFW_KEY_A]) {
      vec3_mul_cross(side, camera->front, camera->up);
      vec3_norm(side, side);
      vec3_scale(side, side, (move_speed*0.9f) * phys_delta_time);
      side[1] = 0.0f;
      vec3_sub(player->velocity, player->velocity, side);
    }
    if (ex_keys_down[GLFW_KEY_D]) {
      vec3_mul_cross(side, camera->front, camera->up);
      vec3_norm(side, side);
      vec3_scale(side, side, (move_speed*0.9f) * phys_delta_time);
      side[1] = 0.0f;
      vec3_add(player->velocity, player->velocity, side);
    }

    // jumping
    if (ex_keys_down[GLFW_KEY_SPACE] && player->grounded == 1) {
      player->velocity[1] = 20.0f;
    }
    
    // update the scene models animation etc
    ex_scene_update(scene, phys_delta_time);

    accumulator -= phys_delta_time;
  }

  // render scene
  ex_scene_draw(scene);

  // woo text
  char str[64];
  sprintf(str, "FPS %i", (int)(1.0 / delta_time));
  ex_text_print(raleway, str, 16, 48, 0.6f, 0.0f, 0.0f, 0.0f, 1.0f, 0.18f, 0.53f);
  ex_text_print(raleway, "exengine-web 0.1", 16, 16, 0.6f, 0.0f, 0.0f, 0.0f, 1.0f, 0.18f, 0.53f);

  ex_window_end();
}

void at_exit()
{
  ex_sound_destroy(sound);
  ex_text_destroy_font(raleway);
}