#version 100

precision mediump float;

varying vec2 uv;

uniform sampler2D u_text;
uniform vec3 u_textcolor;

void main()
{
  gl_FragColor = vec4(1.0, 1.0, 1.0, texture2D(u_text, uv).a) * vec4(u_textcolor, 1.0);
}
