#version 300 es

precision mediump float;

in vec2 uv;

out vec4 color;

uniform sampler2D u_text;
uniform vec3 u_textcolor;

void main()
{
  color = vec4(1.0, 1.0, 1.0, texture(u_text, uv).a) * vec4(u_textcolor, 1.0);
}
