#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "window.h"
#include "text.h"
#include "sound.h"

ex_scene_t* ex_scene_new(GLuint shader)
{
  ex_scene_t *s = malloc(sizeof(ex_scene_t));

  // init lists
  s->model_list   = list_new();
  s->texture_list = list_new();

  s->fps_camera = NULL;

  s->shader = shader;

  // init physics shiz
  memset(s->gravity, 0, sizeof(vec3));
  s->coll_tree = ex_octree_new(OBJ_TYPE_UINT);
  s->coll_list = list_new();
  s->coll_vertices   = NULL;
  s->collision_built = 0;
  s->coll_vertices_last = 0;

  ex_text_init();
  ex_sound_init();

  s->primshader = ex_shader_compile("data/shaders/primshader.vs", "data/shaders/primshader.fs");

  return s;
}

void ex_scene_add_collision(ex_scene_t *s, ex_model_t *model)
{
  if (model != NULL) {
    if (model->vertices != NULL || model->num_vertices == 0) {
      list_add(s->coll_list, (void*)model);
      s->collision_built = 0;

      if (s->coll_vertices != NULL) {
        size_t len = model->num_vertices + s->coll_vertices_last;
        s->coll_vertices = realloc(s->coll_vertices, sizeof(vec3)*len);
        memcpy(&s->coll_vertices[s->coll_vertices_last], &model->vertices[0], sizeof(vec3)*model->num_vertices);
        free(model->vertices);
        s->coll_vertices_last = len;
      } else {
        s->coll_vertices = malloc(sizeof(vec3)*model->num_vertices);
        memcpy(&s->coll_vertices[0], &model->vertices[0], sizeof(vec3)*model->num_vertices);
        s->coll_vertices_last = model->num_vertices;
      }

      model->vertices     = NULL;
      model->num_vertices = 0;
      s->collision_built  = 0;
    }
  }
}

void ex_scene_build_collision(ex_scene_t *s)
{
  // destroy and reconstruct tree
  if (s->coll_tree->built)
    s->coll_tree = ex_octree_reset(s->coll_tree);

  if (s->coll_tree == NULL || s->coll_vertices == NULL || s->coll_vertices_last == 0)
    return;

  ex_rect_t region;
  memcpy(&region.min, &s->coll_tree->region.min, sizeof(vec3));
  memcpy(&region.max, &s->coll_tree->region.max, sizeof(vec3));
  for (int i=0; i<s->coll_vertices_last; i+=3) {
    vec3 tri[3];
    memcpy(tri[0], s->coll_vertices[i+0], sizeof(vec3));
    memcpy(tri[1], s->coll_vertices[i+1], sizeof(vec3));
    memcpy(tri[2], s->coll_vertices[i+2], sizeof(vec3));

    vec3_min(region.min, region.min, tri[0]);
    vec3_min(region.min, region.min, tri[1]);
    vec3_min(region.min, region.min, tri[2]);
    vec3_max(region.max, region.max, tri[0]);
    vec3_max(region.max, region.max, tri[1]);
    vec3_max(region.max, region.max, tri[2]);

    ex_octree_obj_t *obj = malloc(sizeof(ex_octree_obj_t));
    obj->data_uint    = i;
    obj->box          = ex_rect_from_triangle(tri);
    list_add(s->coll_tree->obj_list, (void*)obj);
  }

  memcpy(&s->coll_tree->region, &region, sizeof(ex_rect_t));
  ex_octree_build(s->coll_tree);

  s->collision_built = 1;
}

void ex_scene_update(ex_scene_t *s, float delta_time)
{
  // build collision octree
  if (!s->collision_built)
    ex_scene_build_collision(s);

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
  glEnable(GL_CULL_FACE);
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

  ex_sound_exit();
  ex_text_exit();
}