#ifndef EX_CAMERA_H
#define EX_CAMERA_H

#define GLEW_STATIC
#include <GL/glew.h>

#include "math.h"
#include <stdbool.h>

typedef struct {
  vec3 position, front, up;
  double yaw, pitch, last_x, last_y, sensitivity, fov;
  mat4x4 view, projection;
  int width, height;
} ex_fps_camera_t;

/**
 * [ex_fps_camera_new create a new isometric ortho camera]
 * @param  x [x position]
 * @param  y [y position]
 * @param  z [z position]
 */
ex_fps_camera_t* ex_fps_camera_new(float x, float y, float z, double sensitivity, double fov);

/**
 * [iso_cam_resize reset projections etc]
 * @param cam [ex_fps_camera_t pointer]
 */
void ex_fps_camera_resize(ex_fps_camera_t *cam);

/**
 * [ex_fps_camera_update update the cams projections etc]
 * @param cam            [ex_fps_camera_t pointer]
 * @param shader_program [shader program to use]
 */
void ex_fps_camera_update(ex_fps_camera_t *cam, GLuint shader_program);

void ex_fps_camera_draw(ex_fps_camera_t *cam, GLuint shader_program);

#endif // EX_CAMERA_H