#version 300 es

layout (location = 0) in vec3 in_position;

uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
  gl_Position = u_projection * u_view * vec4(in_position, 1.0f);
}