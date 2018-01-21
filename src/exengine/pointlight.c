#include "pointlight.h"
#include "shader.h"
#include "engine.h"
#include <stdlib.h>
#include <string.h>

#define SHADOW_MAP_SIZE 1024
mat4x4 point_shadow_projection;
GLuint point_light_shader;

GLuint shadow_mat_loc, far_plane_loc, light_pos_loc;
GLuint light_active_loc, lightp_far_loc, lightp_position_loc, lightp_color_loc, is_shadow_loc, depth_loc;
int pointlight_cached = 0, pointlightdepth_cached = 0;

void ex_point_light_init()
{
  // compile the shaders
  point_light_shader = ex_shader_compile("data/shaders/pointlight.vs", "data/shaders/pointlight.fs");

  float aspect = (float)SHADOW_MAP_SIZE/(float)SHADOW_MAP_SIZE;
  mat4x4_perspective(point_shadow_projection, rad(90.0f), aspect, 0.1f, EX_POINT_FAR_PLANE);
}

ex_point_light_t *ex_point_light_new(vec3 pos, vec3 color, int dynamic)
{
  ex_point_light_t *l = malloc(sizeof(ex_point_light_t));

  // set light properties
  memcpy(l->position, pos, sizeof(vec3));
  memcpy(l->color, color, sizeof(vec3));

  if (EXENGINE_GLES == 3) {
    // generate cube map
    glGenTextures(1, &l->depth_map);
    glBindTexture(GL_TEXTURE_CUBE_MAP, l->depth_map);
    for (int i=0; i<6; i++)
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT16,
        SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   #ifndef __EMSCRIPTEN__
    GLfloat border[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, border);
   #endif

    // only want the depth buffer
    glGenFramebuffers(6, &l->depth_map_fbo[0]);
    for (int i=0; i<6; i++) {
      glBindFramebuffer(GL_FRAMEBUFFER, l->depth_map_fbo[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, l->depth_map, 0);
      glDrawBuffers(GL_NONE, 0);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      printf("Error! Point light framebuffer is not complete!\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  l->shader     = point_light_shader;
  l->dynamic    = dynamic;
  l->update     = 1;
  l->is_shadow  = 1;
  l->is_visible = 1;
  return l;
}

void ex_point_light_begin(ex_point_light_t *l, int face)
{
  l->update = 0;

  if (EXENGINE_GLES == 3) {
    vec3 temp;
    vec3_add(temp, l->position, (vec3){1.0f, 0.0f, 0.0f});
    mat4x4_look_at(l->transform[0], l->position, temp, (vec3){0.0f, -1.0f, 0.0f});
    mat4x4_mul(l->transform[0], point_shadow_projection, l->transform[0]);

    vec3_add(temp, l->position, (vec3){-1.0f, 0.0f, 0.0f});
    mat4x4_look_at(l->transform[1], l->position, temp, (vec3){0.0f, -1.0f, 0.0f});
    mat4x4_mul(l->transform[1], point_shadow_projection, l->transform[1]);

    vec3_add(temp, l->position, (vec3){0.0f, 1.0f, 0.0f});
    mat4x4_look_at(l->transform[2], l->position, temp, (vec3){0.0f, 0.0f, 1.0f});
    mat4x4_mul(l->transform[2], point_shadow_projection, l->transform[2]);

    vec3_add(temp, l->position, (vec3){0.0f, -1.0f, 0.0f});
    mat4x4_look_at(l->transform[3], l->position, temp, (vec3){0.0f, 0.0f, -1.0f});
    mat4x4_mul(l->transform[3], point_shadow_projection, l->transform[3]);

    vec3_add(temp, l->position, (vec3){0.0f, 0.0f, 1.0f});
    mat4x4_look_at(l->transform[4], l->position, temp, (vec3){0.0f, -1.0f, 0.0f});
    mat4x4_mul(l->transform[4], point_shadow_projection, l->transform[4]);
   
    vec3_add(temp, l->position, (vec3){0.0f, 0.0f, -1.0f});
    mat4x4_look_at(l->transform[5], l->position, temp, (vec3){0.0f, -1.0f, 0.0f});
    mat4x4_mul(l->transform[5], point_shadow_projection, l->transform[5]);

    // render to depth cube map
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    glBindFramebuffer(GL_FRAMEBUFFER, l->depth_map_fbo[face]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, l->depth_map, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(l->shader);

    if (!pointlightdepth_cached) {
      shadow_mat_loc = glGetUniformLocation(l->shader, "u_shadow_matrice");
      far_plane_loc = glGetUniformLocation(l->shader, "u_far_plane");
      light_pos_loc = glGetUniformLocation(l->shader, "u_light_pos");
      pointlightdepth_cached = 1;
    }

    // pass transform matrices to shader
    glUniformMatrix4fv(shadow_mat_loc, 1, GL_FALSE, *l->transform[face]);
    glUniform1f(far_plane_loc, EX_POINT_FAR_PLANE);
    glUniform3fv(light_pos_loc, 1, l->position);
  }
}

void ex_point_light_draw(ex_point_light_t *l, GLuint shader, const char *prefix)
{
  if (!pointlight_cached) {
    light_active_loc = glGetUniformLocation(shader,  "u_point_active");
    lightp_far_loc = glGetUniformLocation(shader,  "u_point_light.far");
    lightp_position_loc = glGetUniformLocation(shader, "u_point_light.position");
    lightp_color_loc = glGetUniformLocation(shader, "u_point_light.color");
    is_shadow_loc = glGetUniformLocation(shader, "u_point_light.is_shadow");
    depth_loc = glGetUniformLocation(shader, "u_point_depth");
    pointlight_cached = 1;
  }

  if (l->is_shadow && EXENGINE_GLES == 3) {
    glUniform1i(is_shadow_loc, 1);
    glUniform1i(depth_loc, 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, l->depth_map);
  } else if (prefix != NULL) {
    char buff[64];
    sprintf(buff, "%s.is_shadow", prefix);
    glUniform1i(glGetUniformLocation(shader, buff), 0);
  }

  if (prefix != NULL) {
    char buff[64];
    sprintf(buff, "%s.far", prefix);
    glUniform1f(glGetUniformLocation(shader,  buff), EX_POINT_FAR_PLANE);
    sprintf(buff, "%s.position", prefix);
    glUniform3fv(glGetUniformLocation(shader, buff), 1, l->position);
    sprintf(buff, "%s.color", prefix);
    glUniform3fv(glGetUniformLocation(shader, buff), 1, l->color);
  } else {
    glUniform1i(light_active_loc, 1);
    glUniform1f(lightp_far_loc, EX_POINT_FAR_PLANE);
    glUniform3fv(lightp_position_loc, 1, l->position);
    glUniform3fv(lightp_color_loc, 1, l->color);
  }
}

void ex_point_light_destroy(ex_point_light_t *l)
{
  if (EXENGINE_GLES == 3) {
    glDeleteFramebuffers(6, &l->depth_map_fbo[0]);
    glDeleteTextures(1, &l->depth_map);
  }
  free(l);
}