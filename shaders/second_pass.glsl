#version 450

layout(local_size_x = 32, local_size_y = 32) in;

layout(rgba8, binding = 0) uniform image2D out_image;
layout(r32f, binding = 1) uniform image2D in_image_approx_depth;

#include "camera.glsl"

#include "ray_tracing_voxel_world.glsl"

const int FIRST_PASS_CELL_SIZE = 8;

vec3 sample_color(ivec2 pixel_coords, vec2 pixel_offset) {
    vec3 dir = GetCameraRayDirection(pixel_coords, pixel_offset);
    vec3 origin = GetCameraOrigin();

    float distance;
    if (!WorldHitBox(origin, dir, distance)) {
        return vec3(0);
    }

    //dist = WorldConeCast(origin + dir*(distance + 1e-4), dir, 0.005);
    ivec2 cone_coords = pixel_coords / FIRST_PASS_CELL_SIZE;

    RayCastResult res = WorldRayCast(origin + dir * (0 * VOXEL_SIZE + distance + 1e-4), dir, 200);

    vec3 light = normalize(vec3(0.2, 1.0, 0.35));
    if (res.hit) {
        float l = max(0, dot(light, res.normal)) * 0.8 + 0.2f;
        float r = float(res.voxel_data & 0x0000FFu) / 255.0f;
        float g = float((res.voxel_data & 0x00FF00u) >> 8) / 255.0f;
        float b = float((res.voxel_data & 0xFF0000u) >> 16) / 255.0f;
        float c = ((res.cell.x ^ res.cell.y ^ res.cell.z) & 32) > 0 ? 0.8 : 1.0;
        float ch = ((res.cell.x ^ res.cell.y ^ res.cell.z) & 512) > 0 ? 0.8 : 1.0;

        //return vec3(res.iterations) / 32.0;
        return vec3(r, g, b) * l;
    }
    return vec3(0);
}

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    if (pixel_coords.x >= VIEWPORT.x || pixel_coords.y >= VIEWPORT.y) {
        return;
    }

    vec3 color1 = sample_color(pixel_coords, vec2(0.25, 0.25));
    //vec3 color2 = sample_color(normalized_coords2);
    //vec3 color3 = sample_color(normalized_coords3);
    vec3 color4 = sample_color(pixel_coords, vec2(0.75, 0.75));

    imageStore(out_image, pixel_coords, vec4((color1 /*+ color2 + color3*/ + color4) * 0.5, 1));
}
