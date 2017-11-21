#version 300 es

// vec4 is vec2 position, vec2 texture
layout (location = 0) in vec4 in_vertex;

out vec2 uv;

uniform mat4 u_projection;

void main()
{
  gl_Position = u_projection * vec4(in_vertex.xy, 0.0, 1.0);
  uv = in_vertex.zw;
}