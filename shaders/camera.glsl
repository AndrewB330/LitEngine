
layout(std430, binding = 1) buffer CameraInfo {
    ivec2 VIEWPORT;
    mat4 CAMERA_TRANSFORM;
    mat4 CAMERA_TRANSFORM_INV;
};

const float DISTANCE_TO_PLANE = 1.0;

vec3 GetCameraOrigin() {
    return (CAMERA_TRANSFORM * vec4(0, 0, 0, 1)).xyz;
}

vec3 GetCameraRayDirection(ivec2 pixel_coords, vec2 pixel_offset) {
    vec2 normalized_coords = (2 * vec2(pixel_coords) - vec2(VIEWPORT) + pixel_offset * 2) / VIEWPORT.y;
    vec3 dir = normalize(vec3(normalized_coords, DISTANCE_TO_PLANE));
    return (CAMERA_TRANSFORM * vec4(dir, 0)).xyz;
}
