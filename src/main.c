#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <emscripten/emscripten.h>

#include "shader.h"
#include "window.h"
#include "camera.h"
#include "texture.h"

GLFWwindow *window = NULL;
GLuint shader, vbo, vao;
ex_fps_camera_t *camera;
ex_texture_t *wall_tex;

// cube vertices
float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};
// cube model matrix
mat4x4 m;

void do_frame();

int main()
{
  ex_window_init(640, 480, "emscripten");

  shader = ex_shader_compile("data/shader.vs", "data/shader.fs");

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // setup a fps camera
  camera = ex_fps_camera_new(1.5f, 0.0f, 0.5f, 0.1f, 75.0f);
  mat4x4_identity(m);

  // load a texture
  wall_tex = ex_texture_load("wall.png", 0);
  
  // start game loop
  emscripten_set_main_loop(do_frame, 0, 0);
}

void do_frame()
{
  // update
  ex_fps_camera_update(camera, shader);

  // rotate dat cube
  mat4x4_rotate(m, m, 1.0f, 0.5f, 0.2f, 0.01f);

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

  // render shiz
  glClearColor(1.0, 0.5, 0.5, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(shader);
  glDisable(GL_CULL_FACE);

  ex_fps_camera_draw(camera, shader);

  GLuint model_location = glGetUniformLocation(shader, "u_model");
  glUniformMatrix4fv(model_location, 1, GL_FALSE, m[0]);

  // texture
  glBindTexture(GL_TEXTURE_2D, wall_tex->id);

  // render cube
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  glfwSwapBuffers(display.window);
}
