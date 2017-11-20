#include "shader.h"

GLuint ex_shader_compile(const char *vertex_path, const char *fragment_path)
{
  printf("Loading shader files %s and %s\n", vertex_path, fragment_path);

  // load shader files
  char *vertex_source = NULL, *fragment_source = NULL, *geometry_source = NULL;
  vertex_source = io_read_file(vertex_path, "r");
  fragment_source = io_read_file(fragment_path, "r");
  if (vertex_source == NULL || fragment_source == NULL) {
    printf("Failed creating shader\n");
    
    // clean up
    if (vertex_source != NULL)
      free(vertex_source);
    if (fragment_source != NULL)
      free(fragment_source);
    
    return 0;
  }

  // create the shaders
  GLuint vertex_shader, fragment_shader;
  vertex_shader   = glCreateShader(GL_VERTEX_SHADER);
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  // compile the shaders
  glShaderSource(vertex_shader, 1, (const GLchar**)&vertex_source, NULL);
  glCompileShader(vertex_shader);

  GLint success = 0;
  GLchar compile_log[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, 512, NULL, compile_log);
    printf("Failed to compile vertex shader\n%s\n", compile_log);
    goto exit;
  }

  glShaderSource(fragment_shader, 1, (const GLchar**)&fragment_source, NULL);
  glCompileShader(fragment_shader);

  success = 0;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, compile_log);
    printf("Failed to compile fragment shader\n%s\n", compile_log); 
    goto exit;
  }

  // create shader program
  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);

  success = 0;
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, compile_log);
    printf("Failed to link shader program\n%s\n", compile_log);
    goto exit;
  }

exit:
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  if (vertex_source != NULL)
    free(vertex_source);
  if (fragment_source != NULL)
    free(fragment_source);

  printf("Shaders successfully compiled\n");

  return shader_program;
}