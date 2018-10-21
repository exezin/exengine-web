#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "model.h"
#include "pointlight.h"
#include "dirlight.h"
#include "gbuffer.h"
#include "window.h"
#include "sound.h"
#include "ssao.h"

ex_scene_t* ex_scene_new(uint8_t flags)
{
  ex_scene_t *s = malloc(sizeof(ex_scene_t));

  // renderer features
  s->ssao     = 0;
  s->deferred = 0;

  // init lists
  s->model_list = list_new(); 

  // init framebuffers etc
  s->framebuffer = ex_framebuffer_new(0, 0);

  // init lights
  ex_point_light_init();
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++)
    s->point_lights[i] = NULL;

  // init physics shiz
  memset(s->gravity, 0, sizeof(vec3));
  s->coll_tree = ex_octree_new(OBJ_TYPE_UINT);
  s->coll_list = list_new();
  s->coll_vertices   = NULL;
  s->collision_built = 0;
  s->coll_vertices_last = 0;
  memset(s->coll_tree->region.min, 0, sizeof(vec3));
  memset(s->coll_tree->region.max, 0, sizeof(vec3));

  // init debug vars
  s->plightc    = 0;
  s->dynplightc = 0;
  s->dlightc    = 0;
  s->slightc    = 0;
  s->modelc     = 0;

  // load shaders
  s->primshader = ex_shader_compile("primshader.vs", "primshader.fs", NULL);
  s->forwardshader = ex_shader_compile("forward.vs", "forward.fs", NULL);

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

void ex_scene_add_spotlight(ex_scene_t *s, ex_spot_light_t *sl)
{
  if (sl->dynamic && sl->is_shadow)
    s->dynplightc++;
  else
    s->slightc++;

  for (int i=0; i<EX_MAX_SPOT_LIGHTS; i++) {
    if (s->spot_lights[i] == NULL) {
      s->spot_lights[i] = sl;
      return;
    }
  }
}

void ex_scene_update(ex_scene_t *s, float delta_time)
{
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

  // handle light stuffs
  ex_scene_manage_lights(s);
}

void ex_scene_render_depthmaps(ex_scene_t *s)
{
  // render pointlight depth maps
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
}

void ex_scene_draw(ex_scene_t *s, int view_x, int view_y, int view_width, int view_height, ex_camera_matrices_t *matrices)
{
  ex_scene_render_forward(s, view_x, view_y, view_width, view_height, matrices);
}

void ex_scene_render_forward(ex_scene_t *s, int view_x, int view_y, int view_width, int view_height, ex_camera_matrices_t *matrices)
{
  int vw, vh;
  glfwGetFramebufferSize(display.window, &vw, &vh);
  if (!view_width)
    view_width = vw;
  if (!view_height)
    view_height = vh;

  // render light depthmaps
  ex_scene_render_depthmaps(s);

  ex_framebuffer_bind(s->framebuffer);

  // render scene
  glUseProgram(s->forwardshader);

  // send vars to shader
  glUniformMatrix4fv(ex_uniform(s->forwardshader, "u_projection"), 1, GL_FALSE, matrices->projection[0]);
  glUniformMatrix4fv(ex_uniform(s->forwardshader, "u_view"), 1, GL_FALSE, matrices->view[0]);
  glUniformMatrix4fv(ex_uniform(s->forwardshader, "u_inverse_view"), 1, GL_FALSE, matrices->inverse_view[0]);

  // first pass is ambient
  glDisable(GL_BLEND);
  glCullFace(GL_BACK);

  // do all non shadow casting lights in a single pass
  // including the one directional light
  // and lights outside of the shadow render range
  int pcount = 0;
  char buff[64];
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++) {
    ex_point_light_t *pl = s->point_lights[i];
    if (pl == NULL || !pl->is_visible)
      continue;

    if (!pl->is_shadow) {
      sprintf(buff, "u_point_lights[%d]", pcount);
      ex_point_light_draw(pl, s->forwardshader, buff, 0);
      pcount++;
    }
  }

  glUniform1i(ex_uniform(s->forwardshader, "u_point_active"), 0);
  glUniform1i(ex_uniform(s->forwardshader, "u_point_count"), pcount);
  glUniform1i(ex_uniform(s->forwardshader, "u_ambient_pass"), 1);
  ex_scene_render_models(s, s->forwardshader, 0);
  glUniform1i(ex_uniform(s->forwardshader, "u_ambient_pass"), 0);
  glUniform1i(ex_uniform(s->forwardshader, "u_point_count"), 0);

  // enable blending for second pass onwards
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  // render all shadow casting point lights
  for (int i=0; i<EX_SCENE_BIGGEST_LIGHT; i++) {
    ex_point_light_t *pl = i > EX_MAX_POINT_LIGHTS ? NULL : s->point_lights[i];
    
    if (pl == NULL || !pl->is_shadow || !pl->is_visible)
      continue;

    glUniform1i(ex_uniform(s->forwardshader, "u_point_active"), 1);
    ex_point_light_draw(pl, s->forwardshader, NULL, 0);
    ex_scene_render_models(s, s->forwardshader, 0);
  }
  glDisable(GL_BLEND);

  // render debug primitives
  glUseProgram(s->primshader);

  // send vars to shader
  glUniformMatrix4fv(ex_uniform(s->primshader, "u_projection"), 1, GL_FALSE, matrices->projection[0]);
  glUniformMatrix4fv(ex_uniform(s->primshader, "u_view"), 1, GL_FALSE, matrices->view[0]);
  glUniformMatrix4fv(ex_uniform(s->primshader, "u_inverse_view"), 1, GL_FALSE, matrices->inverse_view[0]);

  // render screen quad
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  ex_framebuffer_draw(s->framebuffer, view_x, (vh-view_y-view_height), vw, vh);
}

void ex_scene_manage_lights(ex_scene_t *s)
{
  return;

  // FIX THIS

  // set our position and front vector
  vec3 thispos, thisfront;
  /*if (s->fps_camera != NULL) {
    memcpy(thisfront, s->fps_camera->front, sizeof(vec3));
    memcpy(thispos, s->fps_camera->position, sizeof(vec3));
  }*/

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
    float offset = fmax(fmax(pl->color[0], pl->color[1]), pl->color[2]);
    if (f < 0.1f-(offset*0.5) && pl->distance_to_cam > EX_POINT_FAR_PLANE*offset)
      pl->is_visible = 0;
    else
      pl->is_visible = 1;
  }

  // spot lights
  for (int i=0; i<EX_MAX_SPOT_LIGHTS; i++) {
    ex_spot_light_t *sl = s->spot_lights[i];
    if (sl == NULL)
      continue;

    // direction to light
    vec3 thatpos;
    vec3_sub(thatpos, sl->position, thispos);
    sl->distance_to_cam = vec3_len(thatpos);
    vec3_norm(thatpos, thatpos);
    vec3_norm(thisfront, thisfront);

    // dot to light
    float f = vec3_mul_inner(thisfront, thatpos);

    // check if its behind us and far away
    if (f <= -0.5f && sl->distance_to_cam > EX_SPOT_FAR_PLANE)
      sl->is_visible = 0;
    else
      sl->is_visible = 1;
  }
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

void ex_scene_resize(ex_scene_t *s, int width, int height)
{
  s->framebuffer = ex_framebuffer_resize(s->framebuffer, width, height);

  if (s->deferred)
    ex_gbuffer_resize(width, height);
}

void ex_scene_destroy(ex_scene_t *s)
{
  printf("Cleaning up scene\n");

  // cleanup point lights
  for (int i=0; i<EX_MAX_POINT_LIGHTS; i++) {
    if (s->point_lights[i] != NULL) {
      ex_point_light_destroy(s->point_lights[i]);
    }
  }

  // clean up dir lights
  if (s->dir_light != NULL)
    ex_dir_light_destroy(s->dir_light);

  // cleanup skybox
  if (s->skybox != NULL) {
    ex_skybox_destroy(s->skybox);
  }

  // cleanup framebuffers
  ex_framebuffer_destroy(s->framebuffer);
}