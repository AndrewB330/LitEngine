layout(std430, binding = 2) buffer GlobalInfoBuffer {
    ivec3 WORLD_SIZE;
    ivec3 CHUNK_SIZE;
    ivec3 WORLD_SIZE_LOG;
    ivec3 CHUNK_SIZE_LOG;
    int WORLD_MAX_LOD;
    int CHUNK_MAX_LOD;
    int GRID_LOD_OFFSET[16];
};

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

const float VOXEL_SIZE = 1.0 / 16.0;
const float VOXEL_SIZE_INV = 16.0;

struct RayCastResult {
    bool hit; // has hit any non-zero voxel?
    uint voxel_data;
    vec3 position; // hit position in object-space
    ivec3 cell; // coordinates of the voxel that was hit
    float depth; // distance the ray traveled before the hit
    ivec3 normal; // normal
    int iterations;
};

ivec3 _ApplyInverse(ivec3 cell, ivec3 dims, ivec3 inversed) {
    return (cell * (1 - inversed) + ((dims - 1) - cell) * inversed);
}

vec3 _ApplyInverse(vec3 pos, ivec3 dims, ivec3 inversed) {
    return pos + inversed * (dims - pos * 2);
}

uint _GetChunk(ivec3 cell, int lod) {
    cell >>= lod;
    return buf_world_data[GRID_LOD_OFFSET[lod - CHUNK_MAX_LOD] + (cell.x << (WORLD_SIZE_LOG.y + WORLD_SIZE_LOG.z - lod * 2)) + (cell.y << (WORLD_SIZE_LOG.z - lod)) + cell.z];
}

bool _HasChunk(ivec3 cell, int lod) {
    uint val = _GetChunk(cell, lod);
    return val != 0 && val != 0xFFFFFFFF;
}

uint _GetVoxel(uint chunk, uint bucket, uint chunk_offset, ivec3 cell, int lod) {
    cell = (cell & ((1 << CHUNK_MAX_LOD) - 1)) >> lod;
    return buf_chunk_data[
    chunk_offset
    + (0x249249u & ((0x7FFFFFF8u << (3*(CHUNK_MAX_LOD - lod)))) & ~((0x7FFFFFF8u) << ((CHUNK_MAX_LOD-bucket)*3)))
    + cell.z + (cell.y << (CHUNK_MAX_LOD - lod)) + (cell.x << ((CHUNK_MAX_LOD - lod) << 1))
    ];
}

bool _HasVoxelSlow(ivec3 cell, int lod) {
    if (!all(lessThan(cell, WORLD_SIZE)) || !all(greaterThanEqual(cell, ivec3(0)))) {
        return false;
    }
    if (lod < CHUNK_MAX_LOD) {
        uint chunk = _GetChunk(cell, CHUNK_MAX_LOD);
        if (chunk == 0 || chunk == 0xFFFFFFFF) {
            return false;
        }
        ChunkInfo chunk_info = buf_chunk_info[chunk];
        lod = max(lod, int(chunk_info.bucket));
        return _GetVoxel(chunk, chunk_info.bucket, chunk_info.global_address, cell, lod) > 0;
    }

    return _HasChunk(cell, lod);
}

bool WorldHitBox(vec3 origin, vec3 dir, out float distance) {
    origin = origin * VOXEL_SIZE_INV + WORLD_SIZE / 2;
    vec3 t1 = (-origin) / dir;
    vec3 t2 = (WORLD_SIZE - origin) / dir;
    vec3 tin = min(t1, t2);
    vec3 tout = max(t1, t2);
    float tmin = max(tin.x, max(tin.y, tin.z));
    float tmax = min(tout.x, min(tout.y, tout.z));
    distance = max(tmin, 0) * VOXEL_SIZE;
    return tmax >= 0 && tmin <= tmax;
}

float WorldConeCast(vec3 origin, vec3 dir, float distance, float slope) {
    // Transform to local world coordinates!
    origin = origin * VOXEL_SIZE_INV + WORLD_SIZE / 2;

    float current_radius = max(1.0f, distance * slope * 1.5);

    vec3 initial_origin = origin;

    origin += dir * current_radius;

    int lod = 0;

    float dist_prev = 0.0;

    for(int iteration = 0; iteration < 256; iteration++) {
        float dist = dot(origin - initial_origin, dir);
        ivec3 cell0 = ivec3(origin);

        if (!all(lessThan(cell0, WORLD_SIZE)) || !all(greaterThanEqual(cell0, ivec3(0)))) {
            return length(WORLD_SIZE);
        }

        while ((1 << lod) < current_radius * 2) {
            lod++;
        }
        //lod = max(lod, CHUNK_MAX_LOD);

        ivec3 axis_dirs = ivec3(sign((origin / (1 << lod)) - (cell0 >> lod) - vec3(0.5)));

        ivec3 cell1 = ((cell0 >> lod) + axis_dirs * ivec3(1, 0, 0)) << lod;
        ivec3 cell2 = ((cell0 >> lod) + axis_dirs * ivec3(0, 1, 0)) << lod;
        ivec3 cell3 = ((cell0 >> lod) + axis_dirs * ivec3(1, 1, 0)) << lod;

        ivec3 cell4 = ((cell0 >> lod) + axis_dirs * ivec3(0, 0, 1)) << lod;
        ivec3 cell5 = ((cell0 >> lod) + axis_dirs * ivec3(1, 0, 1)) << lod;
        ivec3 cell6 = ((cell0 >> lod) + axis_dirs * ivec3(0, 1, 1)) << lod;
        ivec3 cell7 = ((cell0 >> lod) + axis_dirs * ivec3(1, 1, 1)) << lod;

        bool any = false;

        any = any || _HasVoxelSlow(cell0, lod);
        any = any || _HasVoxelSlow(cell1, lod);
        any = any || _HasVoxelSlow(cell2, lod);
        any = any || _HasVoxelSlow(cell3, lod);

        any = any || _HasVoxelSlow(cell4, lod);
        any = any || _HasVoxelSlow(cell5, lod);
        any = any || _HasVoxelSlow(cell6, lod);
        any = any || _HasVoxelSlow(cell7, lod);

        if (any) {
            return dist_prev - current_radius;
        }

        float d = (distance + dist) * slope;
        float a = (slope * slope + 1);
        float b = 2 * d * slope;
        float c = d * d - current_radius * current_radius;
        float Dsqr = b * b - 4 * a * c;
        if (Dsqr < 0) {
            // todo: ???
            return 0.0;
        }
        float x = (-b + sqrt(Dsqr)) / a;
        float d2 = d + x * slope;
        current_radius = max(current_radius, d2 * 1.2);
        float y = sqrt(current_radius * current_radius - d2 * d2);
        current_radius = max(current_radius, (d2 + y * slope) * 1.2);
        origin += dir * (x + y);
        dist_prev = dist;
    }

    return dist_prev;
}

RayCastResult WorldRayCast(vec3 origin, vec3 dir, int max_iterations) {
    // Transform to local world coordinates!
    origin = origin * VOXEL_SIZE_INV + WORLD_SIZE * 0.5f;

    // ray_direction should be positive, inverse axes if needed
    ivec3 signs = ivec3(sign(dir));
    // zero -> one
    signs = ivec3(1) * (1 - abs(signs)) + signs;
    ivec3 axes_inversed = (1 - signs) >> 1;

    origin = _ApplyInverse(origin, WORLD_SIZE, axes_inversed);
    dir = normalize(abs(dir) + 1e-5f);

    vec3 ray_direction_inversed = 1.0f / dir; // to speed up division
    vec3 time = vec3(0); // time when we can hit a plane (Y-0-Z, X-0-Z, Y-0-X planes)
    vec3 shifted_ray_origin = origin; // ray origin is shifted each step to reduce floating point errors
    ivec3 cell = ivec3(floor(origin));

    int lod = WORLD_MAX_LOD;
    bool hit = false;

    int iteration = 0;

    // first step, if we are outside of the boundaries
    if (any(lessThan(cell, ivec3(0)))) {
        time = ((cell | ((1 << lod) - 1)) + 1 - shifted_ray_origin) * ray_direction_inversed;
        shifted_ray_origin += ((min(time.x, min(time.y, time.z))) + 1e-4f) * dir;
        cell = ivec3(floor(shifted_ray_origin));
    }

    RayCastResult res;
    int min_bucket = 0;
    for (; iteration < max_iterations && all(lessThan(cell, WORLD_SIZE)); iteration++) {
        ivec3 cell_real = _ApplyInverse(cell, WORLD_SIZE, axes_inversed);

        //while (lod >= CHUNK_MAX_LOD && _HasChunk(cell_real, lod)) lod--;
        lod -= int(lod - 3 >= CHUNK_MAX_LOD && _HasChunk(cell_real, lod - 3)) << 2;
        lod -= int(lod - 1 >= CHUNK_MAX_LOD && _HasChunk(cell_real, lod - 1)) << 1;
        lod -= int(lod >= CHUNK_MAX_LOD && _HasChunk(cell_real, lod));

        if (lod < CHUNK_MAX_LOD) {
            uint chunk_index = _GetChunk(cell_real, CHUNK_MAX_LOD);
            ChunkInfo chunk_info = buf_chunk_info[chunk_index];
            lod = max(lod, max(min_bucket, int(chunk_info.bucket)));
            while (lod > max(chunk_info.bucket, min_bucket) && _GetVoxel(chunk_index, chunk_info.bucket, chunk_info.global_address, cell_real, lod) != 0) {
                lod--;
            }
            if (lod == max(chunk_info.bucket, min_bucket) && (res.voxel_data = _GetVoxel(chunk_index, chunk_info.bucket, chunk_info.global_address, cell_real, lod)) != 0) {
                hit = true;
                break;
            }
        }

        time = ((cell | ((1 << lod) - 1)) + 1 - shifted_ray_origin) * ray_direction_inversed;
        shifted_ray_origin += ((min(time.x, min(time.y, time.z))) + 1e-4f) * dir;

        cell = ivec3(floor(shifted_ray_origin));

        ivec3 bit = findLSB(cell);
        lod = min(max(bit.x, max(bit.y, bit.z)), WORLD_MAX_LOD);
    }
    res.cell = _ApplyInverse(cell, WORLD_SIZE, axes_inversed);
    res.position = (_ApplyInverse(shifted_ray_origin, WORLD_SIZE, axes_inversed) - WORLD_SIZE / 2) * VOXEL_SIZE;
    res.hit = hit;
    res.depth = dot(shifted_ray_origin - origin, dir) * VOXEL_SIZE;
    res.normal = (iteration > 0 ?
        ivec3(step(time.xyz, time.yzx) * step(time.xyz, time.zxy)) :
        ivec3(step(origin.xyz, origin.yzx) * step(origin.xyz, origin.zxy)))
        * (2 * axes_inversed - 1);
    res.iterations = iteration;

    return res;
}