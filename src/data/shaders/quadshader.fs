#version 300 es

precision mediump float;

in vec2 uv;

out vec4 color;

uniform sampler2D u_texture;

void main()
{
  vec3 tex_color = texture(u_texture, uv).rgb;
  color = vec4(tex_color, 1.0);
}