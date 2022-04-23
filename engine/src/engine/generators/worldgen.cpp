#include <lit/engine/generators/worldgen.hpp>
#include <lit/engine/generators/fnl.hpp>
#include <lit/engine/components/voxel_grid/voxel_grid_sparse.hpp>
#include <lit/common/array.hpp>

using namespace lit::engine;
using namespace lit::common;

Array2D<int> computeMinOrMaxInWindow(const Array2D<int> &height, int windowRadius, bool computeMin) {
    auto res = height;
    std::vector<std::deque<std::pair<int, int>>> queues(height.GetWidth());

    auto compare = [&computeMin](int a, int b) {
        return computeMin ? a < b : a > b;
    };

    auto push = [&compare](std::deque<std::pair<int, int>> &q, int index, int value) {
        while (!q.empty() && !compare(q.back().second, value)) {
            q.pop_back();
        }
        q.emplace_back(index, value);
    };

    auto pop = [](std::deque<std::pair<int, int>> &q, int index) {
        while (!q.empty() && q.front().first <= index) {
            q.pop_front();
        }
    };

    for (int i = 0; i < height.GetWidth(); i++) {
        for (int j = 0; j < windowRadius; j++) {
            push(queues[i], j, height.at(i, j));
        }
    }

    for (int j = 0; j < height.GetHeight(); j++) {
        for (int i = 0; i < height.GetWidth(); i++) {
            if (j + windowRadius < height.GetHeight()) {
                push(queues[i], j + windowRadius, height.at(i, j + windowRadius));
            }
            pop(queues[i], j - windowRadius - 1);
        }
        std::deque<std::pair<int, int>> q;
        for (int i = 0; i < windowRadius; i++) {
            push(q, i, queues[i].front().second);
        }

        for (int i = 0; i < height.GetWidth(); i++) {
            if (i + windowRadius < height.GetWidth()) {
                push(q, i + windowRadius, queues[i + windowRadius].front().second);
            }
            pop(q, i - windowRadius - 1);
            res.at(i, j) = q.front().second;
        }
    }

    return res;
}

glm::dvec3 randomVec33(RandomGen &rng) {
    glm::dvec3 v{rng.get_double(-1, 1), rng.get_double(-1, 1), rng.get_double(-1, 1)};

    while (glm::dot(v, v) >= 1.0) {
        v = {rng.get_double(-1, 1), rng.get_double(-1, 1), rng.get_double(-1, 1)};
    }
    return v;
}

Array2D<int> generateHeightMap(size_t width, size_t height, size_t maxHeight) {

    FastNoiseLite noiseMountains(2);
    FastNoiseLite noisePlanes(3);
    FastNoiseLite noiseMix(4);

    noiseMountains.SetFractalOctaves(10);
    noisePlanes.SetFractalOctaves(6);
    noiseMix.SetFractalOctaves(4);

    Array2D<int> heightMap(width, height);

    for (int x = 0; x < width; x++) {
        for (int z = 0; z < height; z++) {
            auto xf = static_cast<float>(x);
            auto zf = static_cast<float>(z);

            float mountains = (noiseMountains.GetNoise(xf, zf, 8192.0f, FastNoiseLite::FractalType::FractalType_FBm) -
                               noiseMountains.GetNoise(xf, zf, 8192.0f, FastNoiseLite::FractalType::FractalType_None) *
                               0.2f)
                              * 1300 + 200.0f;

            float planes =
                    noisePlanes.GetNoise(xf, zf, 8192.0f, FastNoiseLite::FractalType::FractalType_FBm) * 260.0f + 30;

            float mix = noiseMix.GetNoise(xf, zf, 3192.0f, FastNoiseLite::FractalType::FractalType_FBm);
            //mix = mix > 0 ? sqrt(mix) : -sqrt(-mix);
            mix = mix * 6.2f - 1;
            //mix = std::max(0.0f, std::min(mix, 1.0f));
            mix = 1 / (1 + expf(-mix));

            float dx = ((xf / (float) width) - 0.5f) * 2;
            float dz = ((zf / (float) height) - 0.5f) * 2;
            float distFromCenter = dx * dx + dz * dz;
            float drop = powf(distFromCenter + 0.1f, 4) * 300;

            heightMap.at(x, z) = static_cast<int>((mix) * mountains + (1 - mix) * planes - drop);
            //height[x][z] = static_cast<int>(400 * mix + 10);
        }
    }

    return heightMap;
}

void WorldGen::Generate(VoxelGridBaseT<uint32_t> &world, spdlog::logger &logger) {
    logger.trace("Worldgen started");

    auto dimensions = world.GetDimensions();

    auto heightMap = generateHeightMap(dimensions.x, dimensions.z, dimensions.y);

    auto minW = computeMinOrMaxInWindow(heightMap, 12, true);
    auto maxW = computeMinOrMaxInWindow(heightMap, 12, false);


    auto minHeight = computeMinOrMaxInWindow(heightMap, 1, false);

    for (int x = 0; x < dimensions.x; x++) {
        for (int z = 0; z < dimensions.z; z++) {
            for (int y = std::max(0, std::min(minHeight.at(x, z) - 1, heightMap.at(x, z) - 2));
                 y < std::min(heightMap.at(x, z), dimensions.y); y++) {
                world.SetVoxel({x, y, z}, 0x6d6e6d);
            }

            if (maxW.at(x, z) - minW.at(x, z) < 12) {
                world.SetVoxel({x, heightMap.at(x, z), z}, 0x31a312);
            }
        }
    }

    auto w = (VoxelGridSparseT<uint32_t> *) &world;

    logger.trace("Chunks created: {}", w->GetChunksNum());
    logger.trace("World memory size: {}", w->GetSizeBytes());
}

void WorldGen::ResetTestWorld(VoxelGridBaseT<uint32_t> &world) {
    auto dimensions = world.GetDimensions();
    for (int i = 0; i < dimensions.x; i++) {
        for (int k = 0; k < dimensions.z; k++) {
            for (int j = 0; j < dimensions.y; j++) {
                world.SetVoxel({i, j, k}, 0);
            }
            int ii = i / 16;
            int jj = k / 16;
            world.SetVoxel({i, 0, k}, ((ii ^ jj) & 1) ? 0xFFFFFF : 0xF0F0F0);
        }
    }
}

void WorldGen::PlaceObject(VoxelGridBaseT<uint32_t> &world, const VoxelGridBaseT<uint32_t> &object) {
    int offsetX = (world.GetDimensions().x - object.GetDimensions().x) / 2;
    int offsetZ = (world.GetDimensions().z - object.GetDimensions().z) / 2;

    auto dimensions = object.GetDimensions();
    for (int i = 0; i < dimensions.x; i++) {
        for (int k = 0; k < dimensions.z; k++) {
            for (int j = 0; j < dimensions.y; j++) {
                if (object.GetVoxel({i, j, k})) {
                    world.SetVoxel({i + offsetX, j + 1, k + offsetZ}, object.GetVoxel({i, j, k}));
                }
            }
        }
    }
}

std::shared_ptr<lit::engine::VoxelGridBaseT<uint32_t>> GenerateTrunk(RandomGen & rng) {
    auto trunk = std::make_shared<lit::engine::VoxelGridSparseT<uint32_t>>(glm::ivec3(64, 90, 64),
                                                                           glm::dvec3(32, 0, 32));
    auto dimensions = trunk->GetDimensions();
    auto anchor = trunk->GetAnchor();

    auto placeBranch = [&](glm::dvec3 origin, glm::dvec3 direction, double length, double width) {
        width /= 2;
        double highWidth = 0.7 * width;

        for (int i = 0; i < dimensions.x; i++) {
            for (int k = 0; k < dimensions.z; k++) {
                for (int j = 0; j < dimensions.y; j++) {
                    glm::dvec3 v = glm::dvec3{i + 0.5, j + 0.5, k + 0.5} - anchor;

                    double dotA = glm::dot(v - origin, direction);
                    double dotB = glm::dot(v - direction * length - origin, -direction);
                    if (dotA >= 0 && dotB >= 0) {
                        double dist = glm::length(glm::cross(v - origin, direction));
                        double w = width * (1 - dotA / length) + highWidth * dotA / length;
                        if (dist <= w) {
                            trunk->SetVoxel({i, j, k}, 0xFFFFFF);
                        }
                    } else if (dotA < 0) {
                        if (glm::length(v - origin) <= width) {
                            trunk->SetVoxel({i, j, k}, 0xFFFFFF);
                        }
                    } else {
                        if (glm::length(v - direction * length - origin) <= highWidth) {
                            trunk->SetVoxel({i, j, k}, 0xFFFFFF);
                        }
                    }
                }
            }
        }

    };

    double trunkHeight = rng.get_double(20, 40);
    double trunkWidth = rng.get_double(5, 9);
    glm::dvec3 dir = glm::normalize(glm::dvec3(0, 1, 0) + randomVec33(rng) * 0.1);

    placeBranch(glm::dvec3(), dir, trunkHeight, trunkWidth);

    for(int i = 0; i < 5; i++) {
        auto v = randomVec33(rng);
        auto u = glm::normalize(v - glm::dot(v, dir) * dir);
        placeBranch(dir * trunkHeight, glm::normalize(dir + u), trunkHeight * 0.9, trunkWidth * 0.6);
    }


    return trunk;
}

std::shared_ptr<lit::engine::VoxelGridBaseT<uint32_t>> WorldGen::GenerateTree(RandomGen & rng) {

    return GenerateTrunk(rng);

    FastNoiseLite noise(rng.get());
    noise.SetFractalOctaves(8);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);

    auto tree = std::make_shared<lit::engine::VoxelGridSparseT<uint32_t>>(glm::ivec3(64, 64, 64),
                                                                          glm::dvec3(32, 0, 32));


    auto dimensions = tree->GetDimensions();
    for (int i = 0; i < dimensions.x; i++) {
        for (int k = 0; k < dimensions.z; k++) {
            for (int j = 0; j < dimensions.y; j++) {
                float dx = ((float) i - (float) dimensions.x * 0.5f + 0.5f);
                float dy = ((float) j - (float) dimensions.y * 0.5f + 0.5f);
                float dz = ((float) k - (float) dimensions.z * 0.5f + 0.5f);
                float r = sqrt(dx * dx + dy * dy + dz * dz);
                float radius = noise.GetNoise3(dx / r, dy / r, dz / r, 8.0f);
                if (r < (1.0 + radius) * 20) {
                    tree->SetVoxel({i, j, k}, 0x32BB32);
                }
            }
        }
    }

    return tree;
}
