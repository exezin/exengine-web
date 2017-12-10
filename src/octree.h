#ifndef EX_OCTREE_H
#define EX_OCTREE_H

#define GLEW_STATIC
#include <GL/glew.h>

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "list.h"
#include "math.h" 

#define EX_OCTREE_DEFAULT_MIN_SIZE 5.0f
extern int ex_octree_min_size;

enum {
  OBJ_TYPE_UINT,
  OBJ_TYPE_INT,
  OBJ_TYPE_BYTE,
  OBJ_TYPE_FLOAT,
  OBJ_TYPE_DOUBLE,
  OBJ_TYPE_NULL
} ex_ex_octree_obj_type;

typedef struct {
  vec3 min, max;
} ex_rect_t;

typedef struct {
  union {
    uint32_t data_uint;
    int32_t  data_int;
    uint8_t  data_byte;
    float    data_float;
    double   data_double;
  };
  ex_rect_t box;
} ex_octree_obj_t;

typedef struct {
  void *data;
  size_t len;
} ex_octree_data_t;

typedef struct ex_octree_t ex_octree_t;
struct ex_octree_t {
  ex_rect_t region;
  ex_octree_t *children[8];
  int max_life, cur_life;
  list_t *obj_list;
  // flags etc
  uint8_t rendered  : 1;
  uint8_t built     : 1;
  uint8_t first     : 1;
  uint8_t data_type : 5;
  // data
  size_t  data_len;
  union {
    uint32_t *data_uint;
    int32_t  *data_int;
    uint8_t  *data_byte;
    float    *data_float;
    double   *data_double;
  };
  // debug render stuffs
  GLuint vbo, vao, ebo;
  int player_inside;
};

ex_octree_t* ex_octree_new(uint8_t type);

void ex_octree_init(ex_octree_t *o, ex_rect_t region, list_t *objects);

void ex_octree_build(ex_octree_t *o);

void ex_octree_finalize(ex_octree_t *o);

ex_octree_t* ex_octree_reset(ex_octree_t *o);

void ex_octree_get_colliding_count(ex_octree_t *o, ex_rect_t *bounds, int *count);

void ex_octree_get_colliding(ex_octree_t *o, ex_rect_t *bounds, ex_octree_data_t *data_list, int *index);

static inline void* ex_octree_data_ptr(ex_octree_t *o) {
  switch (o->data_type) {
    case OBJ_TYPE_UINT:
      if (o->data_uint != NULL)
        return o->data_uint;
      break;
    case OBJ_TYPE_INT:
      if (o->data_int != NULL)
        return o->data_int;
      break;
    case OBJ_TYPE_BYTE:
      if (o->data_byte != NULL)
        return o->data_byte;
      break;
    case OBJ_TYPE_FLOAT:
      if (o->data_float != NULL)
        return o->data_float;
      break;
    case OBJ_TYPE_DOUBLE:
      if (o->data_double != NULL)
        return o->data_double;
      break;
    default:
      return NULL;
      break;
  }

  return NULL;
}

static inline ex_rect_t ex_rect_new(vec3 min, vec3 max) {
  ex_rect_t r;
  memcpy(r.min, min, sizeof(vec3));
  memcpy(r.max, max, sizeof(vec3));
  return r;
};

static inline float ex_squared(float v) { return v * v; };
static inline int ex_rect_intersect_sphere(ex_rect_t r, vec3 pos, float radius) {
  float dist = radius * radius;
  if (pos[0] < r.min[0]) dist -= ex_squared(pos[0] - r.min[0]);
  else if (pos[0] > r.max[0]) dist -= ex_squared(pos[0] - r.max[0]);
  if (pos[1] < r.min[1]) dist -= ex_squared(pos[1] - r.min[1]);
  else if (pos[1] > r.max[1]) dist -= ex_squared(pos[1] - r.max[1]);
  if (pos[2] < r.min[2]) dist -= ex_squared(pos[2] - r.min[2]);
  else if (pos[2] > r.max[2]) dist -= ex_squared(pos[2] - r.max[2]);
  return dist > 0;
};

static inline int ex_aabb_aabb(ex_rect_t a, ex_rect_t b) {
  return (a.min[0] <= b.max[0] &&
          a.max[0] >= b.min[0] &&
          a.min[1] <= b.max[1] &&
          a.max[1] >= b.min[1] &&
          a.min[2] <= b.max[2] &&
          a.max[2] >= b.min[2]);
};

static inline int ex_aabb_inside(ex_rect_t outer, ex_rect_t inner) {
  return (outer.min[0] <= inner.min[0] &&
          outer.max[0] >= inner.max[0] &&
          outer.min[1] <= inner.min[1] &&
          outer.max[1] >= inner.max[1] &&
          outer.min[2] <= inner.min[2] &&
          outer.max[2] >= inner.max[2]);
};

static inline ex_rect_t ex_rect_from_triangle(vec3 tri[3]) {
  ex_rect_t box;

  vec3_min(box.min, tri[0], tri[1]);
  vec3_min(box.min, box.min, tri[2]);

  vec3_max(box.max, tri[0], tri[1]);
  vec3_max(box.max, box.max, tri[2]);

  return box;
};


#endif // EX_OCTREE_H