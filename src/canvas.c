#include "canvas.h"
#include "shader.h"

GLuint shader;

/* -- screen quad -- */
GLuint vao, vbo;
GLfloat vertices[] = {   
  // pos         // uv
  -1.0f,  1.0f,  0.0f, 1.0f,
  -1.0f, -1.0f,  0.0f, 0.0f,
   1.0f, -1.0f,  1.0f, 0.0f,
  -1.0f,  1.0f,  0.0f, 1.0f,
   1.0f, -1.0f,  1.0f, 0.0f,
   1.0f,  1.0f,  1.0f, 1.0f
};

void ex_canvas_init()
{
  // generic quad rendering shaders
  shader = ex_shader_compile("data/shaders/quadshader.vs", "data/shaders/quadshader.fs");

  // use the same vao/vbo for every framebuffer render
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);

  // vertices
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

  // position
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (GLvoid*)0);
  glEnableVertexAttribArray(0);

  // uv
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (GLvoid*)(2 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

ex_canvas_t* ex_canvas_new(uint32_t width, uint32_t height, GLint internalformat, GLenum format)
{
  printf("Creating canvas with dimensions %i %i\n", width, height);
  ex_canvas_t *c = malloc(sizeof(ex_canvas_t));
  c->width = width;
  c->height = height;

  glGenFramebuffers(1, &c->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, c->fbo);

  // color buffer
  glGenTextures(1, &c->cbo);
  glBindTexture(GL_TEXTURE_2D, c->cbo);
  glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);

  // set params
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#ifndef __EMSCRIPTEN__
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  // how do with webgl2?
  GLfloat border[] = {1.0, 1.0, 1.0, 1.0};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
#else
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif

  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, c->cbo, 0);

  // depth buffer
  glGenRenderbuffers(1, &c->rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, c->rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, c->rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    free(c);
    printf("Error, Framebuffer is not complete\n");
    return NULL;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return c;
}

void ex_canvas_use(ex_canvas_t *c)
{
  glViewport(0, 0, c->width, c->height);
  glBindFramebuffer(GL_FRAMEBUFFER, c->fbo);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
}

void ex_canvas_draw(ex_canvas_t *c, uint32_t width, uint32_t height)
{
  if (width == 0)
    width = c->width;
  if (height == 0)
    height = c->height;

  // clear default color buffer
  glViewport(0, 0, width, height);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glUseProgram(shader);

  // render quad to screen
  glBindVertexArray(vao);
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(glGetUniformLocation(shader, "u_texture"), 0);
  glBindTexture(GL_TEXTURE_2D, c->cbo);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void ex_canvas_destroy(ex_canvas_t *c)
{
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
  glDeleteRenderbuffers(1, &c->rbo);
  glDeleteFramebuffers(1, &c->fbo);
  glDeleteTextures(1, &c->cbo);
  free(c);
  c = NULL;
}