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

ex_font_t* ex_text_load_font(const char *path, uint32_t size)
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

  FT_Set_Pixel_Sizes(face, 0, size);
 
  // force 4-byte alignment
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // calculate atlas size
  int atlas_width = 0, atlas_height = 0;
  for (int i=0; i<128; i++) {
    if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
      printf("Failed to load glyph %i for font face %s\n", i, path);
      continue;
    }

    // get max width and height
    // add 1 for padding
    atlas_width += face->glyph->bitmap.width+1;
    atlas_height = MAX(atlas_height, face->glyph->bitmap.rows);
  }

  // create empty texture for atlas
  glGenTextures(1, &font->atlas);
  glBindTexture(GL_TEXTURE_2D, font->atlas);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  // use GL_ALPHA because GL_RED isnt supported in gles 3.0
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, atlas_width, atlas_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // setup glyphs
  int x = 0;
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, font->atlas);
  for (int i=0; i<128; i++) {
    // load a glyph
    if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
      printf("Failed to load glyph %i for font face %s\n", i, path);
      continue;
    }

    if (!face->glyph->bitmap.buffer)
      continue;

    // add glyph to font atlas
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0,
      face->glyph->bitmap.width, face->glyph->bitmap.rows,
      GL_ALPHA, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

    // setup the character
    font->chars[i].advance[0] = face->glyph->advance.x >> 6;
    font->chars[i].advance[1] = face->glyph->advance.y >> 6;
    font->chars[i].size[0] = face->glyph->bitmap.width;
    font->chars[i].size[1] = face->glyph->bitmap.rows;
    font->chars[i].bearing[0] = face->glyph->bitmap_left;
    font->chars[i].bearing[1] = face->glyph->bitmap_top;
    font->chars[i].xoffset = (float)x / atlas_width;

    // increase for next glyph
    // add 1 for texture padding to prevent bleeding
    x += face->glyph->bitmap.width+1;

  }
  glBindTexture(GL_TEXTURE_2D, 0);

  font->atlas_width = atlas_width;
  font->atlas_height = atlas_height;

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
  glBindTexture(GL_TEXTURE_2D, font->atlas);
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

  // for vertices
  // should probably cache this?
  GLfloat vertices[(6*4) * strlen(str)];
  int vindex = 0;

  // render chars
  for (int i=0; i<strlen(str); i++) {
    ex_char_t ch = font->chars[str[i]];
    
    // nothing to render for whitespace
    if (str[i] == ' ') {
      // move position for next char
      ox += ch.advance[0] * scale;
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

    if (!w || !h)
      continue;

    // add vertices for this quad
    float xoffset = ch.xoffset;
    GLfloat v[6*4] = {
      xpos,     ypos + h, xoffset, 0.0,
      xpos,     ypos,     xoffset, ch.size[1] / font->atlas_height,
      xpos + w, ypos,     xoffset + ch.size[0] / font->atlas_width, ch.size[1] / font->atlas_height,

      xpos,     ypos + h, xoffset, 0.0,
      xpos + w, ypos,     xoffset + ch.size[0] / font->atlas_width, ch.size[1] / font->atlas_height,
      xpos + w, ypos + h, xoffset + ch.size[0] / font->atlas_width, 0.0
    };

    // add quad to vertices array
    memcpy(&vertices[vindex*(6*4)], v, sizeof(v));
    vindex++;

    // move position for next char
    ox += ch.advance[0] * scale;
  }

  // finally render dat string
  glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLES, 0, vindex*6);

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_BLEND);
}

void ex_text_destroy_font(ex_font_t *f)
{
  glDeleteVertexArrays(1, &f->vao);
  glDeleteBuffers(1, &f->vbo);
  free(f);
  f = NULL;
}

void ex_text_exit()
{
  FT_Done_FreeType(ex_text->freetype);
  free(ex_text);
  ex_text = NULL;
}