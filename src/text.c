#include "text.h"
#include "shader.h"
#include "window.h"

ex_text_t *ex_text;
mat4x4 transform;

void ex_text_init()
{
  // init freetype and our text object
  ex_text = malloc(sizeof(ex_text_t));
  if (FT_Init_FreeType(&ex_text->freetype)) {
    printf("Failed to init FreeType\n");
    ex_text = NULL;
    return;
  }

  // load the font render shaders
  ex_text->shader = ex_shader_compile("data/shaders/font.vs", "data/shaders/font.fs");

  // setup projection matrix
  int width, height;
  glfwGetFramebufferSize(display.window, &width, &height);
  mat4x4_ortho(ex_text->projection, 0.0f, width, 0.0f, height, 0.0f, 1.0f);
}

ex_font_t* ex_text_load_font(const char *path)
{
  printf("Loading font face %s\n", path);

  ex_font_t *font = malloc(sizeof(ex_font_t));
  FT_Face face;

  // load the font face
  if (FT_New_Face(ex_text->freetype, path, 0, &face)) {
    printf("Failed to load font face %s\n", path);
    free(font);
    return NULL;
  }

  FT_Set_Pixel_Sizes(face, 0, 48);
 
  // force 4-byte alignment
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // setup glyphs
  for (int i=0; i<128; i++) {
    // load a glyph
    if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
      printf("Failed to load glyph %i for font face %s\n", i, path);
      continue;
    }

    // gen textures
    // use GL_ALPHA because GL_RED isnt supported in gles 3.0
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 
      face->glyph->bitmap.width,
      face->glyph->bitmap.rows,
      0,
      GL_ALPHA,
      GL_UNSIGNED_BYTE,
      face->glyph->bitmap.buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // setup the character
    font->chars[i].id = texture;
    font->chars[i].advance = face->glyph->advance.x;
    font->chars[i].size[0] = face->glyph->bitmap.width;
    font->chars[i].size[1] = face->glyph->bitmap.rows;
    font->chars[i].bearing[0] = face->glyph->bitmap_left;
    font->chars[i].bearing[1] = face->glyph->bitmap_top;

    glBindTexture(GL_TEXTURE_2D, 0);
  }

  FT_Done_Face(face);

  // setup vao and vbo
  glGenVertexArrays(1, &font->vao);
  glGenBuffers(1, &font->vbo);
  glBindVertexArray(font->vao);

  // 6 vertices = 24 floats
  glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 24, NULL, GL_DYNAMIC_DRAW);

  // vertices
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return font;
}

void ex_text_print(ex_font_t *font, const char *str, float x, float y, float scale, float rot, float ox, float oy, float r, float g, float b)
{
  // offset origin
  float dox = ox, doy = oy;
  ox += x;
  oy += y;

  glUseProgram(ex_text->shader);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(font->vao);

  // rotate around origin
  mat4x4_identity(transform);
  mat4x4_translate_in_place(transform, ox, oy, 0.0f);
  mat4x4_rotate_Z(transform, transform, rad(rot));

  // send uniforms
  GLuint loc = glGetUniformLocation(ex_text->shader, "u_textcolor");
  glUniform3f(loc, r, g, b);
  loc = glGetUniformLocation(ex_text->shader, "u_projection");
  glUniformMatrix4fv(loc, 1, GL_FALSE, ex_text->projection[0]);
  loc = glGetUniformLocation(ex_text->shader, "u_text");
  glUniform1i(loc, 0);
  loc = glGetUniformLocation(ex_text->shader, "u_model");
  glUniformMatrix4fv(loc, 1, GL_FALSE, transform[0]);
  loc = glGetUniformLocation(ex_text->shader, "u_origin");
  glUniform2f(loc, ox, oy);

  // render chars
  for (int i=0; i<strlen(str); i++) {
    ex_char_t ch = font->chars[str[i]];
    
    // nothing to render for whitespace
    if (str[i] == ' ') {
      // move position for next char
      ox += (ch.advance >> 6) * scale;
      continue;
    }

    // char position
    float xpos = ox + ch.bearing[0] * scale;
    float ypos = oy - (ch.size[1] - ch.bearing[1]) * scale;
    
    // offset for origin-based scaling
    xpos -= dox * scale;
    ypos -= doy * scale;

    float w = ch.size[0] * scale;
    float h = ch.size[1] * scale;

    // update the vbo
    GLfloat vertices[6][4] = {
      {xpos,     ypos + h, 0.0, 0.0},
      {xpos,     ypos,     0.0, 1.0},
      {xpos + w, ypos,     1.0, 1.0},

      {xpos,     ypos + h, 0.0, 0.0},
      {xpos + w, ypos,     1.0, 1.0},
      {xpos + w, ypos + h, 1.0, 0.0}
    };

    glBindTexture(GL_TEXTURE_2D, ch.id);
    glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // finally render dat char
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // move position for next char
    ox += (ch.advance >> 6) * scale;
  }

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_BLEND);
}