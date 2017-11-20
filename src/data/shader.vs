#version 300 es

in vec3 in_position;
in vec2 in_uv;

out vec2 uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
  gl_Position = u_projection * u_view * u_model * vec4(in_position, 1.0);
  uv = in_uv;
}