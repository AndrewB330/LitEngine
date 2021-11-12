#version 450

layout (location = 0) in vec3 in_pos;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec3 out_material;
layout (location = 2) out vec3 out_normal;
layout (location = 3) out vec3 out_frame_offset;
layout (location = 4) out vec3 out_shadow_position;
layout (location = 5) out vec3 out_direction;

//layout (depth_any) out float gl_FragDepth;

struct VoxelObjectData {
    usampler3D lod_sampler;
    usampler3D data_sampler;
    float voxel_size;
    ivec3 dims;
    int max_lod;
    mat4 transform;
    mat4 transform_inv;
};

uniform VoxelObjectData current_object;

uniform mat4 shadow_transform_inv;

uniform mat4 model_view_matrix_inv;

uniform mat4 projection_matrix_prev;
uniform mat4 view_matrix_prev;
uniform mat4 model_matrix_prev;
uniform vec2 viewport_size;

uniform float z_far;

uniform sampler2D uni_palette;

struct CastResult {
    uint data;
    bool hit; // has hit any non-zero voxel?
    vec3 pos; // hit position in object-space
    ivec3 cell; // coordinates f the voxel that was hit
    float depth; // distance the ray traveled before the hit
    ivec3 normal; // normal
};

ivec3 ApplyInverse(ivec3 cell, ivec3 dims, ivec3 inversed) {
    return (cell * (1 - inversed) + ((dims - 1) - cell) * inversed);
}

vec3 ApplyInverse(vec3 pos, ivec3 dims, ivec3 inversed) {
    return pos + inversed * (dims - pos * 2);
}

bool HasVoxel(VoxelObjectData object, int level, ivec3 cell) {
    cell >>= level;
    int bit = (cell.x & 1) | ((cell.y & 1) << 1) | ((cell.z & 1) << 2);
    return bool((texelFetch(object.lod_sampler, cell >> 1, level).r >> bit) & 1);
}

bool HasVoxelFast(VoxelObjectData object, int level, ivec3 cell) {
    return texelFetch(object.lod_sampler, cell >> level, level - 1).r > 0;
}

uint GetVoxel(VoxelObjectData object, ivec3 cell) {
    return texelFetch(object.data_sampler, cell, 0).r;
}

int LodMask(int lod) {
    return ~((1 << lod) - 1);
}

bool CanLodUp(ivec3 cell, int lod, int maxLod) {
    return lod < maxLod && any(bvec3((cell & LodMask(lod)) == cell));
}

CastResult RayCast(VoxelObjectData object, vec3 ray_origin, vec3 ray_direction, int max_iterations) {
    // ray_direction should be positive, inverse axes if needed
    ivec3 signs = ivec3(sign(ray_direction + 1e-4f));
    ivec3 axes_inversed = (1 - signs) >> 1;

    ray_origin = ApplyInverse(ray_origin, object.dims, axes_inversed);
    ray_direction = normalize(abs(ray_direction) + 1e-6f);

    vec3 ray_direction_inversed = 1.0f / ray_direction; // to speed up division
    vec3 time = vec3(0); // time when we can hit a plane (Y-0-Z, X-0-Z, Y-0-X planes)
    vec3 shifted_ray_origin = ray_origin; // ray origin is shifted each step to reduce floating point errors
    ivec3 cell = ivec3(floor(ray_origin));
    int lod = object.max_lod;
    bool hit = false;

    int iteration = 0;
    for (; iteration < max_iterations && all(lessThan(cell, object.dims)); iteration++) {
        ivec3 cell_real = ApplyInverse(cell, object.dims, axes_inversed);

        while (lod > 0 && HasVoxelFast(object, lod, cell_real)) lod--;
        if (lod == 0 && (hit = HasVoxel(object, lod, cell_real))) break;

        time = ((cell | ((1 << lod) - 1)) + 1 - shifted_ray_origin) * ray_direction_inversed;
        shifted_ray_origin += min(time.x, min(time.y, time.z)) * ray_direction;

        cell = ivec3(shifted_ray_origin);

        if (CanLodUp(cell, lod, object.max_lod)) lod++;
    }

    CastResult res;
    res.hit = hit;
    res.cell = ApplyInverse(cell, object.dims, axes_inversed);
    res.data = GetVoxel(object, res.cell);
    res.depth = dot(shifted_ray_origin - ray_origin, ray_direction);
    res.pos = ApplyInverse(shifted_ray_origin, object.dims, axes_inversed);
    res.normal = (iteration > 0 ?
                  ivec3(step(time.xyz, time.yzx) * step(time.xyz, time.zxy)) :
                  ivec3(step(ray_origin.xyz, ray_origin.yzx) * step(ray_origin.xyz, ray_origin.zxy)))
                 * (2 * axes_inversed - 1);

    return res;
}

void main() {
    vec3 eye_position = (model_view_matrix_inv * vec4(0, 0, 0, 1)).xyz;
    vec3 ray_origin = in_pos;
    vec3 ray_direction = normalize(in_pos - eye_position);
    float initial_depth = length(ray_origin - eye_position);

    bool greater = all(greaterThanEqual(eye_position, vec3(-0.1f)));
    bool less = all(lessThanEqual(eye_position, current_object.dims + vec3(0.1f)));
    if (greater && less) {
        ray_origin = eye_position;
        initial_depth = 0;
    } else {
        ray_origin += ray_direction * 1e-4f; // move ray origin a bit forward
    }

    CastResult mainCast = RayCast(current_object, ray_origin, ray_direction, 256);

    if (!mainCast.hit) {
        out_normal = vec3(0);
        out_color = vec4(0);
        out_frame_offset = vec3(0);
        out_shadow_position = vec3(0);
        gl_FragDepth = 0.999999f;
        return;
    }

    out_normal = normalize(current_object.transform * vec4(mainCast.normal, 0)).xyz;
    out_color = texelFetch(uni_palette, ivec2(mainCast.data - 1, 0), 0);
    //out_color = vec4(vec3(((initial_depth + mainCast.depth) * current_object.voxel_size) / z_far), 1);
    out_color = vec4(vec3(float(GetVoxel(current_object, mainCast.cell))/255.0), 1);
    gl_FragDepth = ((initial_depth + mainCast.depth) * current_object.voxel_size) / z_far;

    if (mainCast.data == 73) {
        out_material = vec3(0.8, 0, 0);
    } else {
        out_material = vec3(0.05, 0, 0);
    }

    out_direction = (current_object.transform * vec4(ray_direction, 0)).xyz;

    vec4 old_pos = projection_matrix_prev * view_matrix_prev * model_matrix_prev * vec4(mainCast.pos, 1);
    out_frame_offset = vec3((old_pos.xy / old_pos.w + 1.0f) * 0.5f - gl_FragCoord.xy / viewport_size, 0);

    out_shadow_position = (shadow_transform_inv * current_object.transform * vec4(mainCast.pos, 1)).xyz;
}
