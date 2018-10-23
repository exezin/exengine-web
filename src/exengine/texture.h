/* texture
  Handles loading and converting
  a texture file to a GL texture.

  Currently uses textures with 4
  components. (rgba)
*/

#ifndef EX_TEXTURE_H
#define EX_TEXTURE_H

#define EX_TEXTURE_LOC "data/textures/"

#include <stdint.h>
#include <stdio.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef struct {
  GLuint id;
  int width, height;
  char name[32];
  uint8_t *data;
} ex_texture_t;

/**
 * [ex_texture_load load a texture from file]
 * @param  file [file path string]
 * @return      [texture var]
 */
ex_texture_t* ex_texture_load(const char *file, int get_data, GLint fa, GLint fb);

#endif // EX_TEXTURE_H