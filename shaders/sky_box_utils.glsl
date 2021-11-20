layout(rgba8, binding = 2) uniform imageCube sky_box;

vec4 GetSkyBoxColor(vec3 direction) {
    direction = normalize(direction);

    vec3 time1 = vec3(-1) / direction;
    vec3 time2 = vec3(1) / direction;

    vec3 toutv = max(time1, time2);
    float tout = min(toutv.x, min(toutv.y, toutv.z));

    vec3 pos = clamp(0.5 - direction * tout * 0.5, 0, 1);

    ivec2 size = imageSize(sky_box);

    vec3 direction_abs = abs(direction);
    if (direction_abs.x > direction_abs.y && direction_abs.x > direction_abs.z) {
        if (direction.x > 0) {
            return imageLoad(sky_box, ivec3(size.x * pos.z, size.y * pos.y, 0));
        } else {
            return imageLoad(sky_box, ivec3(size.x * (1 - pos.z), size.y * pos.y, 1));
        }
    }
    if (direction_abs.y > direction_abs.x && direction_abs.y > direction_abs.z) {
        if (direction.y > 0) {
            return imageLoad(sky_box, ivec3(size.x * (1 - pos.x), size.y * (1 - pos.z), 2));
        } else {
            return imageLoad(sky_box, ivec3(size.x * (1 - pos.x), size.y * pos.z, 3));
        }
    }
    if (direction.z > 0) {
        return imageLoad(sky_box, ivec3(size.x * (1 - pos.x), size.y * pos.y, 4));
    } else {
        return imageLoad(sky_box, ivec3(size.x * pos.x, size.y * pos.y, 5));
    }
}
