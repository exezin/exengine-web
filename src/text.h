#ifndef EX_TEXT_H
#define EX_TEXT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#define GLEW_STATIC
#include <GL/glew.h>

#include "math.h"

typedef struct {
  FT_Library freetype;
  GLuint shader;
  mat4x4 projection;
} ex_text_t;

extern ex_text_t *ex_text;

typedef struct {
  vec2 size, bearing, advance;
  float xoffset;
} ex_char_t;

typedef struct {
  ex_char_t chars[128];
  GLuint vao, vbo, atlas;
  int atlas_width, atlas_height;
} ex_font_t;

void ex_text_init();

ex_font_t* ex_text_load_font(const char *path, uint32_t);

void ex_text_print(ex_font_t *font, const char *str, float x, float y, float scale, float rot, float ox, float oy, float r, float g, float b);

void ex_text_destroy_font(ex_font_t *f);

void ex_text_exit();

#endif // EX_TEXT_H