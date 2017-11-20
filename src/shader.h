#ifndef EX_SHADER_H
#define EX_SHADER_H

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "io.h"

/**
 * [ex_shader_compile loads and compiles a vertex fragment shader]
 * @param  vertex_path   [vertex shader file path]
 * @param  fragment_path [fragment shader file path]
 * @return               [the shader program GLuint]
 */
GLuint ex_shader_compile(const char *vertex_path, const char *fragment_path);


#endif // EX_SHADER_H