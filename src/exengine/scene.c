#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "window.h"
#include "text.h"
#include "sound.h"

GLuint point_count_loc, ambient_loc, point_active_loc;
int scene_cached = 0;

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

  s->plightc = 0;
  s->dynplightc = 0;

  ex_text_init();
  ex_sound_init();
  ex_point_light_init();
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++)
    s->point_lights[i] = NULL;

  s->primshader = ex_shader_compile("data/shaders/primshader.vs", "data/shaders/primshader.fs");


  int w, h;
  glfwGetFramebufferSize(display.window, &w, &h);
  ex_canvas_init();
  s->canvas = ex_canvas_new(w, h, GL_RGBA, GL_RGBA);

  return s;
}

void ex_scene_add_collision(ex_scene_t *s, ex_model_t *model)
{
  if (model != NULL) {
    if (model->vertices != NULL && model->num_vertices > 0) {
      list_add(s->coll_list, (void*)model);
      s->collision_built = 0;

      if (s->coll_vertices != NULL) {
        size_t len = model->num_vertices + s->coll_vertices_last;
        s->coll_vertices = realloc(s->coll_vertices, sizeof(vec3)*len);
        memcpy(&s->coll_vertices[s->coll_vertices_last], &model->vertices[0], sizeof(vec3)*model->num_vertices);
        s->coll_vertices_last = len;
      } else {
        s->coll_vertices = malloc(sizeof(vec3)*model->num_vertices);
        memcpy(&s->coll_vertices[0], &model->vertices[0], sizeof(vec3)*model->num_vertices);
        s->coll_vertices_last = model->num_vertices;
      }

      free(model->vertices);
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
  memcpy(&region, &s->coll_tree->region, sizeof(ex_rect_t));
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

void ex_scene_add_pointlight(ex_scene_t *s, ex_point_light_t *pl)
{
  if (pl == NULL)
    return;
  
  if (pl->dynamic && pl->is_shadow)
    s->dynplightc++;
  else
    s->plightc++;

  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++) {
    if (s->point_lights[i] == NULL) {
      s->point_lights[i] = pl;
      return;
    }
  }

  printf("Maximum point lights exceeded!\n");
}

void ex_scene_update(ex_scene_t *s, float delta_time)
{
  // build collision octree
  if (!s->collision_built)
    ex_scene_build_collision(s);

  if (s->fps_camera)
    ex_fps_camera_update(s->fps_camera, s->shader);

  // update models animations etc
  list_node_t *n = s->model_list;
  while (n->data != NULL) {
    ex_model_update(n->data, delta_time);

    if (n->next != NULL)
      n = n->next;
    else
      break;
  }

  // handle light stuffs
  ex_scene_manage_lights(s);
}

void ex_scene_draw(ex_scene_t *s)
{
  // render pointlight depth maps
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++) {
    ex_point_light_t *l = s->point_lights[i];
    if (l != NULL && (l->dynamic || l->update) && l->is_shadow && l->is_visible) {
      for (int f=0; f<6; f++) {
        ex_point_light_begin(l, f);
        ex_scene_render_models(s, l->shader, 1);
      }
    }
  }

  // main shader render pass
  glUseProgram(s->shader);
  glDisable(GL_BLEND);

  // enable main canvas
  ex_canvas_use(s->canvas);

  if (s->fps_camera != NULL) {
    ex_fps_camera_draw(s->fps_camera, s->shader);
  }
  
  // do all non shadow casting lights in a single pass
  // including the one directional light
  // and lights outside of the shadow render range
  int pcount = 0;
  char buff[64];
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++) {
    ex_point_light_t *pl = s->point_lights[i];
    if (pl == NULL || !pl->is_visible)
      continue;

    if (!pl->is_shadow || pl->distance_to_cam > EX_POINT_SHADOW_DIST) {
      sprintf(buff, "u_point_lights[%d]", pcount);
      ex_point_light_draw(pl, s->shader, buff);
      pcount++;
    }
  }

  if (!scene_cached) {
    point_count_loc = glGetUniformLocation(s->shader, "u_point_count");
    ambient_loc = glGetUniformLocation(s->shader, "u_ambient_pass");
    point_active_loc = glGetUniformLocation(s->shader, "u_point_active");
    scene_cached = 1;
  }
  glUniform1i(point_count_loc, pcount);
  
  // do ambient pass/non shadow casting lighting
  glUniform1i(ambient_loc, 1);
  glUniform1i(point_active_loc, 0);
  ex_scene_render_models(s, s->shader, 0);
  
  // blend everything after here
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glUniform1i(ambient_loc, 0);
  glUniform1i(point_count_loc, 0);
  
  // render all shadow casting point lights
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++) {
    ex_point_light_t *pl = i > EX_MAX_POINT_LIGHTS ? NULL : s->point_lights[i];
    
    if (pl == NULL)
      continue;

    // point light
    if (pl->is_shadow && pl->distance_to_cam <= EX_POINT_SHADOW_DIST && pl->is_visible) {
      glUniform1i(point_active_loc, 1);
      ex_point_light_draw(pl, s->shader, NULL);

      // one render pass for the light and shadows
      ex_scene_render_models(s, s->shader, 0);
    } else {
      glUniform1i(point_active_loc, 0);
    }
  }
  glDisable(GL_BLEND);

  int w, h;
  glfwGetFramebufferSize(display.window, &w, &h);
  ex_canvas_draw(s->canvas, w, h);
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

void ex_scene_manage_lights(ex_scene_t *s)
{
  // set our position and front vector
  vec3 thispos, thisfront;
  if (s->fps_camera != NULL) {
    memcpy(thisfront, s->fps_camera->front, sizeof(vec3));
    memcpy(thispos, s->fps_camera->position, sizeof(vec3));
  }

  // point lights
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++) {
    ex_point_light_t *pl = s->point_lights[i];
    if (pl == NULL)
      continue;

    // direction to light
    vec3 thatpos;
    vec3_sub(thatpos, pl->position, thispos);
    pl->distance_to_cam = vec3_len(thatpos);
    vec3_norm(thatpos, thatpos);
    vec3_norm(thisfront, thisfront);

    // dot to light
    float f = vec3_mul_inner(thisfront, thatpos);

    // check if its behind us and far away
    if (f <= -0.5f && pl->distance_to_cam > EX_POINT_FAR_PLANE)
      pl->is_visible = 0;
    else
      pl->is_visible = 1;
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

  ex_canvas_destroy(s->canvas);

  ex_sound_exit();
  ex_text_exit();
}