#ifndef EX_CANVAS_H
#define EX_CANVAS_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <inttypes.h>

typedef struct {
  GLuint fbo, rbo, cbo;
  uint32_t width, height;
} ex_canvas_t;

void ex_canvas_init();

ex_canvas_t* ex_canvas_new(uint32_t width, uint32_t height, GLint internalformat, GLenum format);

void ex_canvas_use(ex_canvas_t *c);

void ex_canvas_draw(ex_canvas_t *c, uint32_t width, uint32_t height);

void ex_canvas_destroy(ex_canvas_t *c);

#endif // EX_CANVAS_H