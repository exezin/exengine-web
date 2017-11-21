#version 300 es

// vec4 is vec2 position, vec2 texture
layout (location = 0) in vec4 in_vertex;

out vec2 uv;

uniform mat4 u_projection;
uniform mat4 u_model;
uniform vec2 u_origin;

void main()
{
  gl_Position = u_projection * u_model * vec4(in_vertex.xy - u_origin, 0.0, 1.0);
  uv = in_vertex.zw;
}