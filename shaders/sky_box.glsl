#version 450

layout(local_size_x = 32, local_size_y = 32) in;

layout(rgba32f, binding = 0) uniform image2D out_image;
layout(r32f, binding = 1) uniform image2D inout_depth;

#include "camera.glsl"

#include "sky_box_utils.glsl"

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    if (pixel_coords.x >= VIEWPORT.x || pixel_coords.y >= VIEWPORT.y) {
        return;
    }

    vec3 dir = GetCameraRayDirection(pixel_coords, vec2(0.5, 0.5));

    imageStore(out_image, pixel_coords, GetSkyBoxColor(dir));
    imageStore(inout_depth, pixel_coords, vec4(1e9f));
}
