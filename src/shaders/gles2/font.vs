#version 100

// vec4 is vec2 position, vec2 texture
attribute vec4 in_vertex;

varying vec2 uv;

uniform mat4 u_projection;
uniform mat4 u_model;
uniform vec2 u_origin;

void main()
{
  gl_Position = u_projection * u_model * vec4(in_vertex.xy - u_origin, 0.0, 1.0);
  uv = in_vertex.zw;
}