#version 450

layout(local_size_x = 32, local_size_y = 32) in;

layout(rgba32f, binding = 0) uniform image2D inout_image;

#include "camera.glsl"

vec3 FilmicToneMapping(vec3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0, 1);
}

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    if (pixel_coords.x >= VIEWPORT.x || pixel_coords.y >= VIEWPORT.y) {
        return;
    }

    vec3 color = imageLoad(inout_image, pixel_coords).rgb;

    color = FilmicToneMapping(color);

    imageStore(inout_image, pixel_coords, vec4(color, 1));
}
