#ifndef EX_SCENE_H
#define EX_SCENE_H

#include "list.h"
#include "texture.h"
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "octree.h"
#include "pointlight.h"
#include "canvas.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define EX_MAX_POINT_LIGHTS 50

typedef struct {
  GLuint shader, primshader;
  list_t *model_list, *texture_list, *coll_list;
  ex_fps_camera_t *fps_camera;
  ex_octree_t *coll_tree;
  ex_point_light_t *point_lights[EX_MAX_POINT_LIGHTS];
  int collision_built;
  vec3 *coll_vertices;
  size_t coll_vertices_last;
  vec3 gravity;
  int modelc, plightc, dynplightc;
  ex_canvas_t *canvas;
} ex_scene_t;

ex_scene_t* ex_scene_new(GLuint shader);

void ex_scene_add_collision(ex_scene_t *s, ex_model_t *model);

void ex_scene_build_collision(ex_scene_t *s);

void ex_scene_add_pointlight(ex_scene_t *s, ex_point_light_t *pl);

void ex_scene_update(ex_scene_t *s, float delta_time);

void ex_scene_draw(ex_scene_t *s);

void ex_scene_render_models(ex_scene_t *s, GLuint shader, int shadows);

void ex_scene_manage_lights(ex_scene_t *s);

GLuint ex_scene_add_texture(ex_scene_t *s, const char *file);

void ex_scene_destroy(ex_scene_t *s);

#endif // EX_SCENE_H