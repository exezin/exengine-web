#version 100

attribute vec2 in_position;
attribute vec2 in_uv;

varying vec2 uv;

void main() 
{
  gl_Position = vec4(in_position.x, in_position.y, 0.0, 1.0);
  uv = in_uv;
}