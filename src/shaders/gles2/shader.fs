#version 100

precision mediump float;

varying vec2 uv;
varying vec3 frag;
varying vec3 normals;

uniform vec3 u_view_position;
uniform bool u_ambient_pass;
uniform sampler2D u_texture;

/* point light */
const int MAX_PL = 150;
struct point_light {
  vec3 position;
  vec3 color;
  bool is_shadow;
  float far;
};
// for dynamic lights
uniform point_light u_point_light;
uniform samplerCube u_point_depth;
// for static ones done in a single render pass
uniform point_light u_point_lights[MAX_PL];
uniform int         u_point_count;
uniform bool        u_point_active;
/* ------------ */

vec3 calc_point_light(point_light l)
{
  // point light
  vec3 fragpos = frag;
  vec3 diff    = texture2D(u_texture, uv).rgb;
  vec3 norm    = normalize(normals);

  vec3 view_dir  = normalize(u_view_position - fragpos);
  float distance = length(l.position - fragpos);
  vec3 light_dir = normalize(l.position - fragpos);
  
  // diffuse
  vec3 diffuse   = max(dot(light_dir, norm), 0.0) * diff * l.color;

  // attenuation
  float attenuation = 1.0 / (1.0 + 0.1*distance + (0.01*distance*distance));
  diffuse  *= attenuation;

  return vec3((1.0) * diffuse);
}

void main()
{
  if (u_ambient_pass) {
    gl_FragColor = texture2D(u_texture, uv) * 0.1;

    // non shadow casters
    if (u_point_count > 0)
      for (int i=0; i<MAX_PL; i++) {
        if (i == u_point_count)
          break;
        
        gl_FragColor += vec4(calc_point_light(u_point_lights[i]), 1.0);
      }
  
  } else {
    vec3 diffuse = vec3(0.0);

    if (u_point_active && u_point_count <= 0)
      diffuse += calc_point_light(u_point_light);

    gl_FragColor = vec4(diffuse, 1.0);
  }
}
