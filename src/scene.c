#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "model.h"
#include "window.h"

ex_scene_t* ex_scene_new(GLuint shader)
{
  ex_scene_t *s = malloc(sizeof(ex_scene_t));

  // init lists
  s->model_list   = list_new();
  s->texture_list = list_new();

  s->fps_camera = NULL;

  s->shader = shader;

  return s;
}

void ex_scene_update(ex_scene_t *s, float delta_time)
{
  // update models animations etc
  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    ex_model_update(n->data, delta_time);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
}

void ex_scene_draw(ex_scene_t *s)
{
  // main shader
  glUseProgram(s->shader);
  glDisable(GL_BLEND);
  glCullFace(GL_BACK);

  // update camera
  if (s->fps_camera != NULL) {
    ex_fps_camera_update(s->fps_camera, s->shader);
    ex_fps_camera_draw(s->fps_camera, s->shader);
  }

  // render scene models
  ex_scene_render_models(s, s->shader, 0);
}

void ex_scene_render_models(ex_scene_t *s, GLuint shader, int shadows)
{
  s->modelc = 0;
  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    ex_model_t *m = (ex_model_t*)n->data;
    s->modelc++;

    if ((shadows && m->is_shadow) || !shadows)
      ex_model_draw(m, shader);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }
}

GLuint ex_scene_add_texture(ex_scene_t *s, const char *file)
{  
  // check if texture already exists
  list_node_t *n = s->texture_list;
  while (n->data != NULL) {
    ex_texture_t *t = n->data;

    // compare file names
    if (strcmp(file, t->name) == 0) {
      // yep, return that one
      return t->id;
    }

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // doesnt exist, create texture
  ex_texture_t *t = ex_texture_load(file, 0);
  if (t != NULL) {
    // store it in the list
    list_add(s->texture_list, (void*)t);
    return t->id;
  }

  return 0;
}

void ex_scene_destroy(ex_scene_t *s)
{
  printf("Cleaning up scene\n");

  // cleanup models
  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    ex_model_destroy(n->data);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // free model list
  list_destroy(s->model_list);

  // cleanup textures
  n = s->texture_list;
  while (n->data != NULL) {
    ex_texture_t *t = n->data;
    
    // free texture data
    glDeleteTextures(1, &t->id);
    free(t);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // free texture list
  list_destroy(s->texture_list);

  // cleanup cameras
  if (s->fps_camera != NULL)
    free(s->fps_camera);
}