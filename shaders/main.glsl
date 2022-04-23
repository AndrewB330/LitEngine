#version 450

layout(local_size_x = 32, local_size_y = 32) in;

layout(rgba32f, binding = 0) uniform image2D out_image;
layout(r32f, binding = 1) uniform image2D inout_depth;

uniform float uni_time;

#include "camera.glsl"

#include "ray_tracing_voxel_world.glsl"

#include "sky_box_utils.glsl"

#include "water.glsl"

vec3 SampleColor(ivec2 pixel_coords, vec2 pixel_offset, inout float depth) {
    vec3 dir = GetCameraRayDirection(pixel_coords, pixel_offset);
    vec3 origin = GetCameraOrigin();

    float distance;
    if (!WorldHitBox(origin, dir, distance)) {
        return vec3(0);
    }

    RayCastResult res = WorldRayCast(origin + dir * (distance + 1e-6), dir, 200);

    vec3 light = normalize(vec3(1.3, 1.0, 0.35));
    if (res.hit) {
        float l = max(0, dot(light, res.normal)) * 0.7 + 0.3f;
        float r = float((res.voxel_data & 0xFF0000u) >> 16) / 255.0f;
        float g = float((res.voxel_data & 0x00FF00u) >> 8) / 255.0f;
        float b = float(res.voxel_data & 0x0000FFu) / 255.0f;

        float c = ((res.cell.x ^ res.cell.y ^ res.cell.z) & 32) > 0 ? 0.8 : 1.0;
        float ch = ((res.cell.x ^ res.cell.y ^ res.cell.z) & 512) > 0 ? 0.8 : 1.0;

        RayCastResult light_res = WorldRayCast(res.position + res.normal * 1e-5f, light, 200);

        depth = res.depth + distance;

        if (res.cell.y % 32 < 16) {
            //g /= 2;
        }

        if (light_res.hit) {
            return vec3(r, g, b) * 0.2f;
        }

        return vec3(r, g, b) * l;
    }

    return vec3(0);
}

vec3 SampleWaterColor(ivec2 pixel_coords, vec2 pixel_offset, inout float depth) {
    vec3 dir = GetCameraRayDirection(pixel_coords, pixel_offset);
    vec3 origin = GetCameraOrigin();

    if (dir.y >= -0.03) {
        return vec3(0);
    }

    return GetWater(origin + VOXEL_SIZE * WORLD_SIZE.y * 0.5 - 0.5, dir, depth);
}

void main() {
    int warp_index = int(gl_LocalInvocationID.y);
    int thread_index = int(gl_LocalInvocationID.x);
    ivec2 offset = ivec2((warp_index & 3) * 8, (warp_index >> 2) * 4);
    ivec2 offset2 = ivec2((thread_index & 7), (thread_index >> 3));
    //ivec2 pixel_coords = ivec2(gl_WorkGroupID * gl_WorkGroupSize) + offset + offset2;
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    if (pixel_coords.x >= VIEWPORT.x || pixel_coords.y >= VIEWPORT.y) {
        return;
    }

    float prev_depth = 1e9;

    float depth = prev_depth;

    vec3 sum = vec3(0);
    int num = 0;

    float cur_depth = prev_depth;
    vec3 color = SampleColor(pixel_coords, vec2(0.25, 0.25), cur_depth);
    if (cur_depth < prev_depth) {
        num++;
        sum += color;
        depth = min(depth, cur_depth);
    }
    /*cur_depth = prev_depth;
    color = SampleColor(pixel_coords, vec2(0.75, 0.25), cur_depth);
    if (cur_depth < prev_depth) {
        num++;
        sum += color;
        depth = min(depth, cur_depth);
    }
    cur_depth = prev_depth;
    color = SampleColor(pixel_coords, vec2(0.25, 0.75), cur_depth);
    if (cur_depth < prev_depth) {
        num++;
        sum += color;
        depth = min(depth, cur_depth);
    }
    cur_depth = prev_depth;
    color = SampleColor(pixel_coords, vec2(0.75, 0.75), cur_depth);
    if (cur_depth < prev_depth) {
        num++;
        sum += color;
        depth = min(depth, cur_depth);
    }*/


    if (depth < prev_depth) {
        imageStore(out_image, pixel_coords, vec4(sum / num, 1));
        imageStore(inout_depth, pixel_coords, vec4(depth));
        prev_depth = depth;
    }

    //color = SampleWaterColor(pixel_coords, vec2(0.5, 0.5), depth);

    if (depth < prev_depth) {
        imageStore(out_image, pixel_coords, vec4(color, 1));
        imageStore(inout_depth, pixel_coords, vec4(depth));
        prev_depth = depth;
    }
}
