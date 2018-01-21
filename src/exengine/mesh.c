#include "mesh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GLuint diffuse_loc[2], spec_loc[2], norm_loc[2], is_lit_loc[2], is_texture_loc[2], is_spec_loc[2], is_norm_loc[2], transform_loc[2];
GLuint mesh_cached[2] = {0, 0};

ex_mesh_t* ex_mesh_new(ex_vertex_t* vertices, size_t vcount, GLuint *indices, size_t icount, GLuint texture)
{
  ex_mesh_t* m = malloc(sizeof(ex_mesh_t));

  m->texture = texture;
  m->texture_spec = 0;
  m->texture_norm = 0;
  m->vcount  = vcount;
  m->icount  = icount;
  m->is_lit  = 1;
  m->scale   = 1.0f;

  memset(m->position, 0, sizeof(vec3));
  memset(m->rotation, 0, sizeof(vec3));

  mat4x4_identity(m->transform);

  glGenVertexArrays(1, &m->VAO);
  glGenBuffers(1, &m->VBO);
  glGenBuffers(1, &m->EBO);

  glBindVertexArray(m->VAO);

  // vertices
  glBindBuffer(GL_ARRAY_BUFFER, m->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(ex_vertex_t)*m->vcount, &vertices[0], GL_STATIC_DRAW);

  // indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*m->icount, &indices[0], GL_STATIC_DRAW);

  // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ex_vertex_t), (GLvoid*)0);
  glEnableVertexAttribArray(0);

  // tex coords
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ex_vertex_t), (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  // normals
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ex_vertex_t), (GLvoid*)(5 * sizeof(GLfloat)));
  glEnableVertexAttribArray(2);

  // tangents
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ex_vertex_t), (GLvoid*)(8 * sizeof(GLfloat)));
  glEnableVertexAttribArray(3);

  // color
  glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ex_vertex_t), (GLvoid*)(12 * sizeof(GLfloat)));
  glEnableVertexAttribArray(4);

  // blend indexes
  glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ex_vertex_t), (GLvoid*)(12 * sizeof(GLfloat)+(4 * sizeof(GLubyte))));
  glEnableVertexAttribArray(5);

  // blend weights
  glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ex_vertex_t), (GLvoid*)(12 * sizeof(GLfloat)+(8 * sizeof(GLubyte))));
  glEnableVertexAttribArray(6);

  glBindVertexArray(0);

  return m;
}

void ex_mesh_draw(ex_mesh_t* m, GLuint shader_program)
{
  // handle transformations
  if (!m->use_transform) {
    mat4x4_identity(m->transform);
    mat4x4_translate_in_place(m->transform, m->position[0], m->position[1], m->position[2]);
    mat4x4_rotate_Y(m->transform, m->transform, rad(m->rotation[1]));
    mat4x4_rotate_X(m->transform, m->transform, rad(m->rotation[0]));
    mat4x4_rotate_Z(m->transform, m->transform, rad(m->rotation[2]));
    mat4x4_scale_aniso(m->transform, m->transform, m->scale, m->scale, m->scale);
  }

  // bind vao/ebo/tex
  glBindVertexArray(m->VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->EBO);

  int index = 0;
  if (mesh_cached[0] == shader_program) {
    index = 0;
  } else if (mesh_cached[1] == shader_program) {
    index = 1;
  } else {
    if (!mesh_cached[0]) {
      mesh_cached[0] = shader_program;
      index = 0;
    } else if (!mesh_cached[1]) {
      mesh_cached[1] = shader_program;
      index = 1;
    }

    diffuse_loc[index] = glGetUniformLocation(shader_program, "u_texture");
    spec_loc[index] = glGetUniformLocation(shader_program, "u_spec");
    norm_loc[index] = glGetUniformLocation(shader_program, "u_norm");
    is_lit_loc[index] = glGetUniformLocation(shader_program, "u_is_lit");
    is_texture_loc[index] = glGetUniformLocation(shader_program, "u_is_textured");
    is_spec_loc[index] = glGetUniformLocation(shader_program, "u_is_spec");
    is_norm_loc[index] = glGetUniformLocation(shader_program, "u_is_norm");
    transform_loc[index] = glGetUniformLocation(shader_program, "u_model");
  }



  glUniform1i(diffuse_loc[index], 4);
  glUniform1i(spec_loc[index], 5);
  glUniform1i(norm_loc[index], 6);
  glUniform1i(is_lit_loc[index], m->is_lit);
  
  if (m->texture < 1) {
    glUniform1i(is_texture_loc[index], 0);
  } else { 
    glUniform1i(is_texture_loc[index], 1);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m->texture);
  }

  if (m->texture_spec < 1) {
    glUniform1i(is_spec_loc[index], 0);
  } else {
    glUniform1i(is_spec_loc[index], 1);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m->texture_spec);
  }

  if (m->texture_norm < 1) {
    glUniform1i(is_norm_loc[index], 0);
  } else {
    glUniform1i(is_norm_loc[index], 1);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m->texture_norm);
  }

  // pass transform matrix to shader
  glUniformMatrix4fv(transform_loc[index], 1, GL_FALSE, m->transform[0]);

  // draw mesh
  glDrawElements(GL_TRIANGLES, m->icount, GL_UNSIGNED_INT, 0);

  // unbind buffers
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void ex_mesh_destroy(ex_mesh_t* m)
{
  glDeleteVertexArrays(1, &m->VAO);
  glDeleteBuffers(1, &m->VBO);
  glDeleteBuffers(1, &m->EBO);

  free(m);
  m = NULL;
}