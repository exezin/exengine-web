#version 100

attribute vec3 in_position;
attribute vec2 in_uv;
attribute vec3 in_normals;
attribute vec4 in_tangents;
attribute vec4 in_color;
attribute vec4 in_boneindex;
attribute vec4 in_boneweights;

varying vec4 frag;

uniform mat4 u_model;
uniform mat4 u_bone_matrix[200];
uniform bool u_has_skeleton;
uniform mat4 u_shadow_matrice;

void main()
{
	mat4 transform = u_model;
  if (u_has_skeleton == true) {
    mat4 skeleton = u_bone_matrix[int(in_boneindex.x*255.0)] * in_boneweights.x +
                    u_bone_matrix[int(in_boneindex.y*255.0)] * in_boneweights.y +
                    u_bone_matrix[int(in_boneindex.z*255.0)] * in_boneweights.z +
                    u_bone_matrix[int(in_boneindex.w*255.0)] * in_boneweights.w;

    transform = u_model * skeleton;
  }

	gl_Position = u_shadow_matrice * transform * vec4(in_position, 1.0);
  frag = transform * vec4(in_position, 1.0);
}