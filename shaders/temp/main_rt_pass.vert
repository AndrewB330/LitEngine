#version 450

layout (location = 0) in vec3 in_vert;

layout (location = 0) out vec3 out_pos;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

uniform ivec3 voxel_object_dims;

void main() {
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(voxel_object_dims * in_vert, 1);
    out_pos = voxel_object_dims * in_vert;
}
