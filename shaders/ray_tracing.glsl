#version 450

layout(local_size_x = 32, local_size_y = 32) in;

layout(rgba8, binding = 0) uniform image2D out_image;

layout(location = 1) uniform ivec2 uni_viewport;
layout(location = 2) uniform mat4 uni_camera_transform;
layout(location = 3) uniform mat4 uni_camera_transform_inv;

layout(location = 4) uniform ivec3 uni_world_size;
layout(location = 5) uniform ivec3 uni_chunk_size;

layout(location = 6) uniform int uni_world_max_lod;
layout(location = 7) uniform int uni_chunk_max_lod;

layout(location = 20) uniform int uni_world_lod_buf_offset[10];
layout(location = 30) uniform int uni_chunk_lod_buf_offset[10];
layout(location = 40) uniform ivec2 uni_world_linearizer[10];
layout(location = 50) uniform ivec2 uni_chunk_linearizer[10];

layout(std430, binding = 16) buffer WorldDataBuffer {
    uint buf_world_data[];
};
layout(std430, binding = 17) buffer ChunkDataBuffer {
    uint buf_chunk_data[];
};

struct ChunkInfo {
    uint global_address;
    uint bucket;
};

layout(std430, binding = 18) buffer ChunkInfoBuffer {
    ChunkInfo buf_chunk_info[];
};

const float distance_to_plane = 1.0;
const float voxel_size = 1.0 / 16.0;

bool hit_world(vec3 origin, vec3 dir, out float distance) {
    origin = origin / voxel_size + uni_world_size / 2;
    vec3 t1 = (-origin) / dir;
    vec3 t2 = (uni_world_size - origin) / dir;
    vec3 tin = min(t1, t2);
    vec3 tout = max(t1, t2);
    float tmin = max(tin.x, max(tin.y, tin.z));
    float tmax = min(tout.x, min(tout.y, tout.z));
    distance = max(tmin, 0) * voxel_size;
    return tmax >= 0 && tmin <= tmax;
}

struct RayCastResult {
    uint data;
    bool hit; // has hit any non-zero voxel?
    vec3 pos; // hit position in object-space
    ivec3 cell; // coordinates f the voxel that was hit
    float depth; // distance the ray traveled before the hit
    ivec3 normal; // normal
    int iter;
    uint bucket;
};

ivec3 apply_inverse(ivec3 cell, ivec3 dims, ivec3 inversed) {
    return (cell * (1 - inversed) + ((dims - 1) - cell) * inversed);
}

vec3 apply_inverse(vec3 pos, ivec3 dims, ivec3 inversed) {
    return pos + inversed * (dims - pos * 2);
}

uint get_chunk(ivec3 cell, int lod) {
    cell >>= lod;
    ivec2 linearizer = uni_world_linearizer[lod - uni_chunk_max_lod];
    return buf_world_data[uni_world_lod_buf_offset[lod - uni_chunk_max_lod] + cell.x * linearizer.y + cell.y * linearizer.x + cell.z];
}

bool has_chunk(ivec3 cell, int lod) {
    uint val = get_chunk(cell, lod);
    return val != 0 && val != 0xFFFFFFFF;
}

uint get_chunk_voxel(uint chunk, uint bucket, uint chunk_offset, ivec3 cell, int lod) {
    cell = (cell & ((1 << uni_chunk_max_lod) - 1)) >> lod;
    return buf_chunk_data[
    chunk_offset
    + (0x249249u & ((0x7FFFFFF8u << (3*(uni_chunk_max_lod - lod)))) & ~((0x7FFFFFF8u) << ((uni_chunk_max_lod-bucket)*3)))
    + cell.z + (cell.y << (uni_chunk_max_lod - lod)) + (cell.x << ((uni_chunk_max_lod - lod) << 1))
    ];
}

int lod_mask(int lod) {
    return ~((1 << lod) - 1);
}

bool can_lod_up(ivec3 cell, int lod, int maxLod) {
    return lod < maxLod && any(equal((cell & lod_mask(lod)), cell));
}

RayCastResult ray_cast_world(vec3 origin, vec3 dir, int max_iterations) {
    origin = origin / voxel_size + uni_world_size / 2;

    // ray_direction should be positive, inverse axes if needed
    ivec3 signs = ivec3(sign(dir + 1e-4f));
    ivec3 axes_inversed = (1 - signs) >> 1;

    origin = apply_inverse(origin, uni_world_size, axes_inversed);
    dir = normalize(abs(dir) + 1e-5f);

    vec3 ray_direction_inversed = 1.0f / dir; // to speed up division
    vec3 time = vec3(0); // time when we can hit a plane (Y-0-Z, X-0-Z, Y-0-X planes)
    vec3 shifted_ray_origin = origin; // ray origin is shifted each step to reduce floating point errors
    ivec3 cell = ivec3(floor(origin));

    int lod = 9;
    bool hit = false;

    int iteration = 0;

    RayCastResult res;
    for (; iteration < max_iterations && all(lessThan(cell, uni_world_size)); iteration++) {
        ivec3 cell_real = apply_inverse(cell, uni_world_size, axes_inversed);
        while (lod >= uni_chunk_max_lod && has_chunk(cell_real, lod)) lod--;
        if (lod < uni_chunk_max_lod) {
            uint chunk_index = get_chunk(cell_real, uni_chunk_max_lod);
            ChunkInfo chunk_info = buf_chunk_info[chunk_index];
            lod = max(lod, int(chunk_info.bucket));
            while (lod > chunk_info.bucket && get_chunk_voxel(chunk_index, chunk_info.bucket, chunk_info.global_address, cell_real, lod) != 0) {
                lod--;
            }
            if (lod == chunk_info.bucket && get_chunk_voxel(chunk_index, chunk_info.bucket, chunk_info.global_address, cell_real, lod) != 0) {
                hit = true;
                res.bucket = chunk_info.bucket;
                break;
            }
        }

        time = ((cell | ((1 << lod) - 1)) + 1 - shifted_ray_origin) * ray_direction_inversed;
        shifted_ray_origin += (min(time.x, min(time.y, time.z)) + 1e-4) * dir;

        cell = ivec3(shifted_ray_origin);

        ivec3 bit = findLSB(cell);
        lod = min(max(bit.x, max(bit.y, bit.z)), uni_world_max_lod);
    }
    res.iter = iteration;
    res.hit = hit;
    res.data = 1;
    res.depth = dot(shifted_ray_origin - origin, dir);
    res.normal = ivec3(step(time.xyz, time.yzx) * step(time.xyz, time.zxy)) * (2 * axes_inversed - 1);

    return res;
}

void main() {

    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    if (pixel_coords.x >= uni_viewport.x || pixel_coords.y >= uni_viewport.y) {
        return;
    }
    vec2 normalized_coords = (2 * vec2(pixel_coords) - vec2(uni_viewport) + vec2(1)) / uni_viewport.y;
    vec3 dir = normalize(vec3(normalized_coords, distance_to_plane));
    vec3 origin = vec3(0);

    dir = (uni_camera_transform * vec4(dir, 0)).xyz;
    origin = (uni_camera_transform * vec4(origin, 1)).xyz;

    float distance;
    if (!hit_world(origin, dir, distance)) {
        imageStore(out_image, pixel_coords, vec4(0.8));
        return;
    }

    RayCastResult res = ray_cast_world(origin + dir * (distance + 1e-4), dir, 200);

    vec3 light = normalize(vec3(0.2, 1.0, 0.35));
    if (res.hit) {
        float l = max(0, dot(light, res.normal)) * 0.8 + 0.2f;
        vec3 color = vec3(0.2, 1.0, 0.3);
        if (res.bucket == 1) {
            color = vec3(0.5, 0.8, 0.1);
        }
        if (res.bucket == 2) {
            color = vec3(0.8, 0.5, 0.1);
        }
        if (res.bucket == 3) {
            color = vec3(0.9, 0.3, 0.1);
        }
        if (res.bucket == 4) {
            color = vec3(0.9, 0.4, 0.1);
        }
        if (res.bucket == 5) {
            color = vec3(0.9, 0.9, 0.9);
        }
        imageStore(out_image, pixel_coords, vec4(color * l, 1));
    }
}
