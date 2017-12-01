#version 300 es

precision mediump float;

in vec2 uv;
in vec3 frag;
in vec3 normals;

out vec4 color;

uniform vec3 u_view_position;
uniform bool u_ambient_pass;
uniform sampler2D u_texture;

/* point light */
const int MAX_PL = 50;
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

/*vec3 pcf_offset[20] = vec3[]
(
  vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
  vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
  vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
  vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
  vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);*/

vec3 calc_point_light(point_light l)
{
  // point light
  vec3 fragpos = frag;
  vec3 diff    = texture(u_texture, uv).rgb;
  vec3 norm    = normalize(normals);

  vec3 view_dir  = normalize(u_view_position - fragpos);
  float distance = length(l.position - fragpos);
  vec3 light_dir = normalize(l.position - fragpos);
  
  // diffuse
  vec3 diffuse   = max(dot(light_dir, norm), 0.0) * diff * l.color;

  // attenuation
  float attenuation = 1.0f / distance;
  diffuse  *= attenuation;

  /* shadows AFIJ)FGHWAFHWA FFFFF
  float shadow = 0.0f;
  if (l.is_shadow) {
    float costheta = clamp(dot(norm, light_dir), 0.0, 1.0);
    float bias     = 0.2*tan(acos(costheta));
    bias           = clamp(bias, 0.1, 0.2);
    
    vec3 frag_to_light  = fragpos - l.position;
    float current_depth = length(frag_to_light);
    float view_dist     = length(u_view_position - fragpos);

    // PCF smoothing
    float radius = (1.0 + (view_dist / l.far)) / l.far;
    float offset = 0.1;
    int   samples = 20;
    float closest_depth = 0.0f;
    for (int i=0; i<samples; ++i) {
      closest_depth  = texture(u_point_depth, frag_to_light + pcf_offset[i] * radius).r;
      closest_depth *= l.far;
      if (current_depth > closest_depth)
        shadow += 1.0;
    }
    shadow /= float(samples);

    // closest_depth = texture(u_point_depth, frag_to_light).r;
    // closest_depth *= l.far;
    // shadow = current_depth > closest_depth ? 1.0 : 0.0;
    // return vec3(closest_depth/l.far);
  }*/

  return vec3(diffuse);
}

void main()
{
  color = texture(u_texture, uv) * 0.2f;

  // non shadow casters
  if (u_point_count > 0)
    for (int i=0; i<u_point_count; i++)
      color += vec4(calc_point_light(u_point_lights[i]), 1.0f);
 
  // ALL FUCKED
  /*if (u_ambient_pass) {
    color = texture(u_texture, uv) * 0.0f;

    // non shadow casters
    if (u_point_count > 0)
      for (int i=0; i<u_point_count; i++)
        color += vec4(calc_point_light(u_point_lights[i]), 1.0f);
  
  } else {
    vec3 diffuse = vec3(0.0);

    // if (u_point_active && u_point_count <= 0)
      diffuse += calc_point_light(u_point_light);

    color = vec4(diffuse, 1.0);
  }*/
}
