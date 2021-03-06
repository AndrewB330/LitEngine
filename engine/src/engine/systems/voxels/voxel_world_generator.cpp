#include <lit/engine/systems/voxels/voxel_world_generator.hpp>
#include <random>

using namespace lit::engine;

std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

const uint32_t SIZE_2D = 1024;
const uint32_t SIZE_3D = 256;
float random_sample_2d[SIZE_2D][SIZE_2D][2];
float random_sample_3d[SIZE_3D][SIZE_3D][SIZE_3D][3];

uint32_t Vec3ToColorCode(glm::vec3 color) {
    color = glm::clamp(color, 0.0f, 1.0f);
    return ((int)(color.x * 255)) | ((int)(color.y * 255) << 8) | ((int)(color.z * 255) << 16);
}

uint32_t IVec3ToColorCode(glm::ivec3 color) {
    color = glm::clamp(color, 0, 255);
    return (color.x) | (color.y << 8) | (color.z << 16);
}

/*glm::vec3 HslToRgb(float h, float s, float l) {

    h = fmodf(h + 360.0f, 360.0f);

    float c = (1 - fabs(2 * l - 1)) * s;

}*/

void ResetRandom2D(uint32_t seed) {
    std::mt19937 rng(seed);
    for (int i = 0; i < SIZE_2D; i++) {
        for (int j = 0; j < SIZE_2D; j++) {
            float u = distribution(rng);
            float v = distribution(rng);
            if (u * u + v * v > 1.0f) {
                j--;
                continue;
            }
            float l = sqrt(u * u + v * v);
            random_sample_2d[i][j][0] = u / l;
            random_sample_2d[i][j][1] = v / l;
        }
    }
}

void ResetRandom3D(uint32_t seed) {
    std::mt19937 rng(seed);
    for (int i = 0; i < SIZE_3D; i++) {
        for (int j = 0; j < SIZE_3D; j++) {
            for (int k = 0; k < SIZE_3D; k++) {
                float u = distribution(rng);
                float v = distribution(rng);
                float r = distribution(rng);
                if (u * u + v * v + r * r > 1.0f) {
                    k--;
                    continue;
                }
                float l = sqrt(u * u + v * v + r * r);
                random_sample_3d[i][j][k][0] = u / l;
                random_sample_3d[i][j][k][1] = v / l;
                random_sample_3d[i][j][k][2] = r / l;
            }
        }
    }
}

inline glm::vec2 Gradient2D(glm::ivec2 point) {
    point &= SIZE_2D - 1;
    float u = random_sample_2d[point.x][point.y][0];
    float v = random_sample_2d[point.x][point.y][1];
    return glm::vec2(u, v);
}

inline glm::vec3 Gradient3D(glm::ivec3 point) {
    point &= SIZE_3D - 1;
    float u = random_sample_3d[point.x][point.y][point.z][0];
    float v = random_sample_3d[point.x][point.y][point.z][1];
    float r = random_sample_3d[point.x][point.y][point.z][2];
    return glm::vec3(u, v, r);
}

inline float Fade(float t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float SamplePerlinNoise2D(glm::vec2 p) {
    glm::ivec2 p0 = floor(p);
    glm::ivec2 p1 = p0 + glm::ivec2(1, 0);
    glm::ivec2 p2 = p0 + glm::ivec2(0, 1);
    glm::ivec2 p3 = p0 + glm::ivec2(1, 1);

    glm::vec2 g0 = Gradient2D(p0);
    glm::vec2 g1 = Gradient2D(p1);
    glm::vec2 g2 = Gradient2D(p2);
    glm::vec2 g3 = Gradient2D(p3);

    float fade_x = Fade(p.x - p0.x);
    float fade_y = Fade(p.y - p0.y);

    float p0p1 = (1.0 - fade_x) * dot(g0, (p - glm::vec2(p0))) + fade_x * dot(g1, (p - glm::vec2(p1)));
    float p2p3 = (1.0 - fade_x) * dot(g2, (p - glm::vec2(p2))) + fade_x * dot(g3, (p - glm::vec2(p3)));

    return (1.0 - fade_y) * p0p1 + fade_y * p2p3;
}

float SamplePerlinNoise3D(glm::vec3 p) {
    glm::ivec3 p0 = floor(p);
    glm::ivec3 p1 = p0 + glm::ivec3(1, 0, 0);
    glm::ivec3 p2 = p0 + glm::ivec3(0, 1, 0);
    glm::ivec3 p3 = p0 + glm::ivec3(1, 1, 0);
    glm::ivec3 p4 = p0 + glm::ivec3(0, 0, 1);
    glm::ivec3 p5 = p0 + glm::ivec3(1, 0, 1);
    glm::ivec3 p6 = p0 + glm::ivec3(0, 1, 1);
    glm::ivec3 p7 = p0 + glm::ivec3(1, 1, 1);

    glm::vec3 g0 = Gradient3D(p0);
    glm::vec3 g1 = Gradient3D(p1);
    glm::vec3 g2 = Gradient3D(p2);
    glm::vec3 g3 = Gradient3D(p3);
    glm::vec3 g4 = Gradient3D(p4);
    glm::vec3 g5 = Gradient3D(p5);
    glm::vec3 g6 = Gradient3D(p6);
    glm::vec3 g7 = Gradient3D(p7);

    float fade_x = Fade(p.x - p0.x);
    float fade_y = Fade(p.y - p0.y);
    float fade_z = Fade(p.z - p0.z);


    float p0p1 = (1.0f - fade_x) * dot(g0, (p - glm::vec3(p0))) + fade_x * dot(g1, (p - glm::vec3(p1)));
    float p2p3 = (1.0f - fade_x) * dot(g2, (p - glm::vec3(p2))) + fade_x * dot(g3, (p - glm::vec3(p3)));
    float p4p5 = (1.0f - fade_x) * dot(g4, (p - glm::vec3(p4))) + fade_x * dot(g5, (p - glm::vec3(p5)));
    float p6p7 = (1.0f - fade_x) * dot(g6, (p - glm::vec3(p6))) + fade_x * dot(g7, (p - glm::vec3(p7)));

    float p01p23 = (1.0f - fade_y) * p0p1 + fade_y * p2p3;
    float p45p67 = (1.0f - fade_y) * p4p5 + fade_y * p6p7;

    return (1.0f - fade_z) * p01p23 + fade_z * p45p67;
}

float SampleFractalNoise2D(glm::vec2 p, float wavelength, int levels = 8) {
    float res = 0.0f;
    for (int i = 0; i < levels; i++) {
        float pow = (float)(1 << i);
        res += SamplePerlinNoise2D(p * pow / wavelength) / pow;
    }
    return glm::clamp(res, -1.0f, 1.0f);
}

float SampleFractalNoise3D(glm::vec3 p, float wavelength, int levels = 8) {
    float res = 0.0f;
    for (int i = 0; i < levels; i++) {
        float pow = (float)(1 << i);
        res += SamplePerlinNoise3D(p * pow / wavelength) / pow;
    }
    return glm::clamp(res, -1.0f, 1.0f);
}

glm::ivec3 lerp(glm::ivec3 a, glm::ivec3 b, float t) {
    return glm::ivec3((1 - t) * glm::vec3(a) + t * glm::vec3(b));
}

VoxelGridSparseT<uint32_t> VoxelWorldGenerator::Generate() {
    VoxelGridSparseT<uint32_t> world({512, 512, 512}, {0.0, 0.0, 0.0});
    glm::ivec3 dims = world.GetDimensions();
    int width = dims.x;
    int depth = dims.z;
    glm::vec2 center{ width / 2, depth / 2 };

    std::vector<std::vector<int>> height(width, std::vector<int>(depth));
    std::vector<std::vector<int>> height_grass(width, std::vector<int>(depth));
    std::vector<std::vector<int>> height_sand(width, std::vector<int>(depth));


    /*FastNoiseLite fn;
    fn.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
    fn.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);*/

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < depth; j++) {
            glm::vec2 pos{ i, j };
            float radius = glm::length(pos - center);
            float x = radius / (width * 0.5);
            float noise = SampleFractalNoise2D(pos, 2000.0f, 9) - SamplePerlinNoise2D(pos / 2000.0f) * 0.3f;
            //float noise = fn.GetNoise(pos.x, pos.y, 1 / 3000.0f, 9) - fn.GetNoise(pos.x, pos.y, 1 / 3000.0f, 1) * 0.3;
            height[i][j] = noise * 700 - x * x * 128;
            height_sand[i][j] = height[i][j] * 0.7 + 8 + SampleFractalNoise2D(pos, 64.0f, 6) * 8;
            height_grass[i][j] = height[i][j] + SampleFractalNoise2D(pos, 3.3f, 2) * 2 + 1;
        }
    }

    std::mt19937 rng;
    std::vector<glm::vec3> crowns;
    std::vector<float> sizes;
    std::uniform_real_distribution<float> xd(0, width - 1);
    std::uniform_real_distribution<float> zd(0, depth - 1);
    std::uniform_real_distribution<float> sized(25, 75);

    for (int i = 0; crowns.size() < 0; i++) {
        glm::vec3 pos{ xd(rng), 0, zd(rng) };
        int h = height[(int)pos.x][(int)pos.z];
        pos.y += h + 4;
        for (auto other : crowns) {
            if (glm::distance(other, pos) < 120) h = 0;
        }
        if (h > 40) {
            crowns.push_back(pos);
            sizes.push_back(sized(rng));
        }
    }

    ResetRandom3D(0);

    
    /*world.SetGenerator([&](glm::ivec3 grid_position, VoxelGridSparseT<uint32_t>::ChunkRaw& chunk) {
        std::vector<glm::vec3> crowns_;
        std::vector<float> sizes_;
        for (int i = 0; i < crowns.size(); i++) {
            if (glm::distance((glm::vec3(grid_position) + 0.5f) * (float)VoxelGridSparseT<uint32_t>::CHUNK_SIZE, crowns[i]) < 200) {
                crowns_.push_back(crowns[i]);
                sizes_.push_back(sizes[i]);
            }
        }
        for (int x = 0; x < VoxelGridSparseT<uint32_t>::CHUNK_SIZE; x++) {
            for (int z = 0; z < VoxelGridSparseT<uint32_t>::CHUNK_SIZE; z++) {
                int gx = x + grid_position.x * VoxelGridSparseT<uint32_t>::CHUNK_SIZE;
                int gz = z + grid_position.z * VoxelGridSparseT<uint32_t>::CHUNK_SIZE;
                int h = height[gx][gz];
                int hs = height_sand[gx][gz];
                int h2 = std::max(h, height_grass[gx][gz]);

                const int radius = 3;
                int min = h;
                int max = h;
                for (int dx = -radius; dx <= radius; dx++) {
                    for (int dz = -radius; dz <= radius; dz++) {
                        if (gx + dx < 0 || gx + dx >= width || gz + dz < 0 || gz + dz >= depth) continue;
                        min = std::min(min, height[gx + dx][gz + dz]);
                        max = std::max(max, height[gx + dx][gz + dz]);
                    }
                }

                const int grass_depth = 7;

                glm::vec3 nearest_crown{ 9999,9999,9999 };
                int index = -1;
                for (int i = 0; i < crowns_.size(); i++) {
                    if (glm::distance(crowns_[i], glm::vec3(gx, h, gz)) < glm::distance(nearest_crown, glm::vec3(gx, h, gz))) {
                        nearest_crown = crowns_[i];
                        index = i;
                    }
                }

                for (int y = 0; y < VoxelGridSparseT<uint32_t>::CHUNK_SIZE; y++) {
                    int gy = y + grid_position.y * VoxelGridSparseT<uint32_t>::CHUNK_SIZE;

                    glm::vec3 pos{ gx, gy, gz };

                    if (index >= 0) {
                        float val = SampleFractalNoise3D(pos, 11, 4) + 0.5;
                        val = (val * 0.3 + 0.7) * sizes_[index];

                        if (val > glm::distance(pos, nearest_crown)) {
                            const glm::ivec3 color1 = { 51, 212, 150 };
                            const glm::ivec3 color2 = { 53, 156, 97 };

                            const glm::ivec3 color3 = { 209, 202, 67 };
                            const glm::ivec3 color4 = { 224, 184, 63 };

                            float noise_a = glm::clamp(SampleFractalNoise3D({ gx, gy, gz }, 7.3f, 4) * 1.8 + 0.5, 0.0,
                                1.0);
                            float noise_b = glm::clamp(SampleFractalNoise3D({ gx, gy, gz }, 9.4f, 4) * 1.8 + 0.5, 0.0,
                                1.0);
                            float noise = glm::clamp(SampleFractalNoise3D({ gx, gy, gz }, 1900.0f, 3) * 1.3 + 0.5, 0.0,
                                1.0);

                            chunk[x][y][z] = IVec3ToColorCode(
                                lerp(lerp(color1, color2, noise_a), lerp(color3, color4, noise_b), noise));
                            continue;
                        }
                    }

                    if (hs > h) {
                        if (gy < h) {
                            chunk[x][y][z] = IVec3ToColorCode({ 255, 238, 130 });
                        }
                        continue;
                    }
                    if (max - min < grass_depth) {
                        if (gy < h2 && gy + grass_depth - (max - min) >= h2) {
                            // grass
                            const glm::ivec3 color1 = { 13, 217, 70 };
                            const glm::ivec3 color2 = { 6, 191, 58 };

                            const glm::ivec3 color3 = { 174, 194, 25 };
                            const glm::ivec3 color4 = { 210, 217, 24 };

                            float noise_a = glm::clamp(SampleFractalNoise3D({ gx, gy, gz }, 4.3f, 3) * 1.2 + 0.5, 0.0,
                                1.0);
                            float noise_b = glm::clamp(SampleFractalNoise3D({ gx, gy, gz }, 3.4f, 3) * 1.2 + 0.5, 0.0,
                                1.0);
                            float noise = glm::clamp(SampleFractalNoise3D({ gx, gy, gz }, 1900.0f, 3) * 1.3 + 0.5, 0.0,
                                1.0);

                            chunk[x][y][z] = IVec3ToColorCode(
                                lerp(lerp(color1, color2, noise_a), lerp(color3, color4, noise_b), noise));
                            continue;
                        }
                    }

                    if (gy < h) {
                        const glm::ivec3 color1 = { 105, 92, 64 };
                        const glm::ivec3 color2 = { 94, 81, 53 };

                        const glm::ivec3 color3 = { 138, 138, 138 };
                        const glm::ivec3 color4 = { 110, 109, 109 };
                        float noise_a = glm::clamp(SampleFractalNoise3D({ gx, gy, gz }, 13.3f, 3) * 1.3 + 0.5, 0.0, 1.0);
                        float noise_b = glm::clamp(SampleFractalNoise3D({ gx, gy, gz }, 16.4f, 3) * 1.3 + 0.5, 0.0, 1.0);
                        float noise = glm::clamp(SampleFractalNoise3D({ gx, gy, gz }, 400.0f, 3) * 2.9 + 0.5, 0.0, 1.0);
                        chunk[x][y][z] = IVec3ToColorCode(
                            lerp(lerp(color1, color2, noise_a), lerp(color3, color4, noise_b), noise));
                    }
                    else {
                        // air
                        continue;
                    }
                }
            }
        }
        });*/

    return world;
}
