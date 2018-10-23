#version 300 es

precision mediump float;

in vec2 uv;

out vec4 color;

uniform sampler2D u_texture;

vec3 aces_tonemap(vec3 x) {
  float a = 2.51f;
  float b = 0.03f;
  float c = 2.43f;
  float d = 0.59f;
  float e = 0.14f;
  return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0f, 1.0f);
}

void main() {
  vec3 tcolor = texture(u_texture, uv).rgb;
  tcolor = vec3(1.0) - exp(tcolor / vec3(0.75f, 0.75f, 0.75f));
  color = vec4(aces_tonemap(tcolor), 1.0f);
  color.rgb = pow(color.rgb, vec3(1.0f / 2.2f));
}