#version 450
#pragma optionNV (unroll all)
layout(local_size_x = 32, local_size_y = 32) in;

layout(rgba8, binding = 0) uniform image2D out_image;
layout(r8ui, binding = 1) uniform uimage3D in_global;

layout(location = 0) uniform int uni_objects_num;
layout(location = 1) uniform ivec2 uni_viewport;
layout(location = 2) uniform mat4 uni_camera_transform;
//layout(location = 3) uniform mat4 uni_camera_transform_inv;

layout(location = 4) uniform ivec3 uni_size;
layout(location = 5) uniform mat4 uni_transform;
layout(location = 6) uniform mat4 uni_transform_inv;

layout(std430, binding = 14) buffer Tree { uint buf_tree[][8]; };

layout(std430, binding = 10) buffer GridSizes { ivec3 buf_grid_size[]; };
layout(std430, binding = 11) buffer GridTransform { mat4 buf_grid_transform[]; };
layout(std430, binding = 12) buffer GridTransformInv { mat4 buf_grid_transform_inv[]; };
layout(std430, binding = 13) buffer GridOffset { ivec3 buf_grid_offset[][10]; };

struct Object {
    ivec3 dims;
    int max_lod;
    mat4 transform;
    mat4 transform_inv;
    ivec3 offset;
    ivec3 offset_lod[10];
};

layout(std430, binding = 10) buffer ObjectsBuffer {
    Object objects[];
};

const float distance_to_plane = 1.0;

/*
uimage3D global_color_texture

MAXN - number of objects
uvec3 offsets[MAXN]; // 4kb
uvec3 sizes[MAXN];
mat4 transforms[MAXN]; //
mat4 inv_transforms[MAXN];

MAXT - number of kd tree nodes
vec3 bbox_mins[MAXT];
vec3 bbox_maxs[MAXT];
int lefts[MAXT];
int rights[MAXT];

*/

bool check_hit_against(int object, vec3 dir, vec3 ray, out float distance) {
    ray = (buf_grid_transform_inv[object] * vec4(ray, 0)).xyz;
    dir = (buf_grid_transform_inv[object] * vec4(dir, 1)).xyz;
    vec3 t1 = (-dir) / ray;
    vec3 t2 = (buf_grid_size[object] - dir) / ray;
    vec3 tin = min(t1, t2);
    vec3 tout = max(t1, t2);
    float tmin = max(tin.x, max(tin.y, tin.z));
    float tmax = min(tout.x, min(tout.y, tout.z));
    distance = max(tmin, 0);
    return tmax >= 0 && tmin <= tmax;
}

bool check_hit_against_w(vec3 dir, vec3 ray, out float distance) {
    ray = (uni_transform_inv * vec4(ray, 0)).xyz;
    dir = (uni_transform_inv * vec4(dir, 1)).xyz;
    vec3 t1 = (-dir) / ray;
    vec3 t2 = (uni_size - dir) / ray;
    vec3 tin = min(t1, t2);
    vec3 tout = max(t1, t2);
    float tmin = max(tin.x, max(tin.y, tin.z));
    float tmax = min(tout.x, min(tout.y, tout.z));
    distance = max(tmin, 0);
    return tmax >= 0 && tmin <= tmax;
}

bool check_hit(vec3 origin, vec3 dir, out float distance, out int object) {
    bool ahy_hit = false;
    for (int i = 0; i < uni_objects_num; i++) {
        float cur_distance;
        if (check_hit_against(i, origin, dir, cur_distance) && (!ahy_hit || cur_distance < distance)) {
            distance = cur_distance;
            object = i;
            ahy_hit = true;
        }
    }
    return ahy_hit;
}

struct RayCastResult {
    uint data;
    bool hit; // has hit any non-zero voxel?
    vec3 pos; // hit position in object-space
    ivec3 cell; // coordinates f the voxel that was hit
    float depth; // distance the ray traveled before the hit
    ivec3 normal; // normal
};

ivec3 apply_inverse(ivec3 cell, ivec3 dims, ivec3 inversed) {
    return (cell * (1 - inversed) + ((dims - 1) - cell) * inversed);
}

vec3 apply_inverse(vec3 pos, ivec3 dims, ivec3 inversed) {
    return pos + inversed * (dims - pos * 2);
}

bool has_voxel(int object, int level, ivec3 cell) {
    cell >>= level;
    int bit = (cell.x & 1) | ((cell.y & 1) << 1) | ((cell.z & 1) << 2);
    uint val = imageLoad(in_global, (cell >> 1) + buf_grid_offset[object][level + 1]).r;
    return bool((val >> bit) & 1u);
}

bool has_voxel_fast(int object, int level, ivec3 cell) {
    return imageLoad(in_global, (cell >> level) + buf_grid_offset[object][level]).r > 0;
}

uint get_voxel(int object, ivec3 cell) {
    return imageLoad(in_global, cell + buf_grid_offset[object][0]).r;
}

int lod_mask(int lod) {
    return ~((1 << lod) - 1);
}

bool can_lod_up(ivec3 cell, int lod, int maxLod) {
    return lod < maxLod && any(bvec3((cell & lod_mask(lod)) == cell));
}

RayCastResult ray_cast_inside(int object, vec3 ray_origin, vec3 ray_direction, int max_iterations) {
    ray_origin = (buf_grid_transform_inv[object] * vec4(ray_origin, 1)).xyz;
    ray_direction = (buf_grid_transform_inv[object] * vec4(ray_direction, 0)).xyz;

    // ray_direction should be positive, inverse axes if needed
    ivec3 signs = ivec3(sign(ray_direction + 1e-4f));
    ivec3 axes_inversed = (1 - signs) >> 1;

    ray_origin = apply_inverse(ray_origin, buf_grid_size[object], axes_inversed);
    ray_direction = normalize(abs(ray_direction) + 1e-6f);

    vec3 ray_direction_inversed = 1.0f / ray_direction; // to speed up division
    vec3 time = vec3(0); // time when we can hit a plane (Y-0-Z, X-0-Z, Y-0-X planes)
    vec3 shifted_ray_origin = ray_origin; // ray origin is shifted each step to reduce floating point errors
    ivec3 cell = ivec3(floor(ray_origin));
    int lod = /*object.max_lod*/5;
    bool hit = false;

    int iteration = 0;
    for (; iteration < max_iterations && all(lessThan(cell, buf_grid_size[object])); iteration++) {
        ivec3 cell_real = apply_inverse(cell, buf_grid_size[object], axes_inversed);

        while (lod > 0 && has_voxel_fast(object, lod, cell_real)) lod--;
        if (lod == 0 && (hit = has_voxel(object, lod, cell_real))) break;

        time = ((cell | ((1 << lod) - 1)) + 1 - shifted_ray_origin) * ray_direction_inversed;
        shifted_ray_origin += min(time.x, min(time.y, time.z)) * ray_direction;

        cell = ivec3(shifted_ray_origin);

        if (can_lod_up(cell, lod, 5)) lod++;
    }

    RayCastResult res;
    res.hit = hit;
    res.cell = apply_inverse(cell, buf_grid_size[object], axes_inversed);
    res.data = get_voxel(object, res.cell);
    res.depth = dot(shifted_ray_origin - ray_origin, ray_direction);
    res.pos = apply_inverse(shifted_ray_origin, buf_grid_size[object], axes_inversed);
    res.normal = (iteration > 0 ?
    ivec3(step(time.xyz, time.yzx) * step(time.xyz, time.zxy)) :
    ivec3(step(ray_origin.xyz, ray_origin.yzx) * step(ray_origin.xyz, ray_origin.zxy)))
    * (2 * axes_inversed - 1);

    return res;
}

uint ray_cast_inside_w(vec3 origin, vec3 dir, out float distance) {
    origin = (uni_transform_inv * vec4(origin, 1)).xyz;
    dir = (uni_transform_inv * vec4(dir, 0)).xyz;

    vec3 origin_ = origin;

    ivec3 signs = ivec3(sign(dir + 1e-5f));
    ivec3 axes_inversed = (1 - signs) >> 1;

    origin = apply_inverse(origin, uni_size, axes_inversed);
    dir = normalize(abs(dir));

    vec3 dir_inv = 1.0f / dir; // to speed up division
    vec3 time = vec3(0);

    uint nodes[10];
    ivec3 offset[10];
    int scales[10];

    int ptr = 0;
    nodes[ptr] = 1;

    for(int i = 0; i < 10; i++) {
        scales[i] = uni_size.x >> i;
    }

    for(int iter = 0; iter < 32 && ptr >= 0; iter++) {
        uint node = nodes[ptr];
        ivec3 cell = ivec3(floor((origin - offset[ptr]) / scales[ptr + 1]));
        offset[ptr + 1] = offset[ptr] + cell * scales[ptr + 1];

        for (int iter2 = 0; iter2 < 4 && all(lessThan(cell, ivec3(2))); iter2++) {
            ivec3 cell_real = (cell * (1 - axes_inversed) + (1 - cell) * axes_inversed);

            int bit = cell_real.x | (cell_real.y << 1) | (cell_real.z << 2);
            if ((buf_tree[node][bit] >> 31) != 0) {
                distance = dot(origin - origin_, dir) / dot(dir, dir);
                return buf_tree[node][bit] & ((1u << 31) - 1);
            } if (buf_tree[node][bit] == 0) {
                time = (cell + 1 - (origin - offset[ptr]) / scales[ptr+1]) * dir_inv * scales[ptr+1];
                origin += (min(time.x, min(time.y, time.z)) + 1e-3) * dir;
                cell = ivec3(floor((origin - offset[ptr]) / scales[ptr + 1]));
                offset[ptr + 1] = offset[ptr] + cell * scales[ptr + 1];
            } else {
                // recur
                ptr++;
                nodes[ptr] = buf_tree[node][bit];
                ptr++;
                break; // todo: weird continue?
            }

        }

        ptr--;

    }
    return 0;
}

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    if (pixel_coords.x >= uni_viewport.x || pixel_coords.y >= uni_viewport.y) {
        return;
    }
    vec2 normalized_coords = (2 * vec2(pixel_coords) - vec2(uni_viewport) + vec2(1)) / uni_viewport.y;
    vec3 ray_dir = normalize(vec3(normalized_coords, distance_to_plane));
    vec3 ray_origin = vec3(0);

    ray_dir = (uni_camera_transform * vec4(ray_dir, 0)).xyz;
    ray_origin = (uni_camera_transform * vec4(ray_origin, 1)).xyz;

    float distance;
    /*if (!check_hit_against_w(ray_origin, ray_dir, distance)) {
        return;
    }

    uint res = ray_cast_inside_w(ray_origin + ray_dir * (distance + 1e-3f), ray_dir, distance);

    if (res == 0) {
        return;
    }

    imageStore(out_image, pixel_coords, vec4(float(res & 255u) / 255.0, float((res >> 8) & 255u) / 255.0, float((res >> 16) & 255u) / 255.0, 1.0));
    */
    if (!check_hit_against(0, ray_origin, ray_dir, distance)) {
        return;
    }

    RayCastResult res = ray_cast_inside(0, ray_origin + (distance + 1e-4) * ray_dir, ray_dir, 256);

    if (!res.hit) {
        return;
    }

    imageStore(out_image, pixel_coords, vec4(1));

}
