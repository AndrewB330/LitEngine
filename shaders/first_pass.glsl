#version 450

layout(local_size_x = 32, local_size_y = 32) in;

layout(r32f, binding = 0) uniform image2D out_image;

#include "camera.glsl"

#include "ray_tracing_voxel_world.glsl"

const int FIRST_PASS_CELL_SIZE = 8;

void main() {
    ivec2 cone_coords = ivec2(gl_GlobalInvocationID.xy);

    ivec2 pixel_coords = cone_coords * FIRST_PASS_CELL_SIZE + FIRST_PASS_CELL_SIZE / 2;

    vec3 dir = GetCameraRayDirection(pixel_coords, vec2(FIRST_PASS_CELL_SIZE % 2) / 2);
    vec3 origin = GetCameraOrigin();

    float distance;
    if (!WorldHitBox(origin, dir, distance)) {
        imageStore(out_image, cone_coords, vec4(0));
    }

    distance += 1e-4;

    float approx_depth = WorldConeCast(origin + dir * distance, dir, distance / VOXEL_SIZE, 0.01);

    imageStore(out_image, cone_coords, vec4(approx_depth));
}
