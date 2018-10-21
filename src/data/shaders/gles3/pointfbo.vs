#version 300 es

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normals;
layout (location = 3) in vec4 in_tangents;
layout (location = 4) in vec4 in_color;
layout (location = 5) in vec4 in_boneindex;
layout (location = 6) in vec4 in_boneweights;
layout (location = 7) in mat4 in_instancematrix;

uniform mat4 u_bone_matrix[200];
uniform bool u_has_skeleton;
uniform mat4 u_shadow_matrix;

out vec4 frag;

void main()
{
	mat4 transform = in_instancematrix;
	if (u_has_skeleton == true) {
		mat4 skeleton = u_bone_matrix[int(in_boneindex.x*255.0f)] * in_boneweights.x +
										u_bone_matrix[int(in_boneindex.y*255.0f)] * in_boneweights.y +
										u_bone_matrix[int(in_boneindex.z*255.0f)] * in_boneweights.z +
										u_bone_matrix[int(in_boneindex.w*255.0f)] * in_boneweights.w;

		transform = in_instancematrix * skeleton;
	}

  gl_Position = u_shadow_matrix * transform * vec4(in_position, 1.0f);
  frag =  transform * vec4(in_position, 1.0f);
}