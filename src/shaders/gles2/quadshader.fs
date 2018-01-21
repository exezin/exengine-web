#version 100

precision mediump float;

varying vec2 uv;

uniform sampler2D u_texture;

void main()
{
  vec3 tex_color = texture2D(u_texture, uv).rgb;
  gl_FragColor = vec4(tex_color, 1.0);
}