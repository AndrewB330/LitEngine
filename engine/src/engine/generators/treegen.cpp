#pragma once

#include <lit/engine/components/voxel_grid/voxel_grid_dense.hpp>
#include <lit/engine/generators/treegen.hpp>

using namespace lit::engine;

glm::dvec3 randomVec3(RandomGen &rng) {
    glm::dvec3 v{rng.get_double(-1, 1), rng.get_double(-1, 1), rng.get_double(-1, 1)};

    while (glm::dot(v, v) >= 1.0) {
        v = {rng.get_double(-1, 1), rng.get_double(-1, 1), rng.get_double(-1, 1)};
    }
    return v;
}

TreeGen::TreeGen(int seed) : rng(seed) {}

VoxelGridPtr TreeGen::GenerateTreeAny() {
    auto grid = std::make_shared<VoxelGridDenseT<uint32_t>>(glm::ivec3(128, 128, 128), glm::dvec3(64, 0, 64));

    auto graph = GenerateTreeGraphAny();

    PlaceBranch(graph, grid);

    return grid;
}

void TreeGen::PlaceBranch(const TreeGen::Branch &branch, const VoxelGridPtr &grid) {
    auto dims = grid->GetDimensions();
    auto anchor = grid->GetAnchor();

    glm::ivec3 min = grid->GetDimensions();
    glm::ivec3 max = glm::ivec3();

    min = glm::ivec3(glm::min(branch.origin + branch.direction * branch.length + anchor, branch.origin + anchor) -
                     (branch.radiusBegin + 2));
    max = glm::ivec3(glm::max(branch.origin + branch.direction * branch.length + anchor, branch.origin + anchor) +
                     (branch.radiusBegin + 2));

    for (int i = std::max(0, min.x); i < std::min(dims.x, max.x); i++) {
        for (int j = std::max(0, min.y); j < std::min(dims.y, max.y); j++) {
            for (int k = std::max(0, min.z); k < std::min(dims.z, max.z); k++) {
                glm::dvec3 v = glm::dvec3{i + 0.5, j + 0.5, k + 0.5} - anchor;

                double dotA = glm::dot(v - branch.origin, branch.direction);
                double dotB = glm::dot(v - branch.direction * branch.length - branch.origin, -branch.direction);
                if (dotA >= 0 && dotB >= 0) {
                    double dist = glm::length(glm::cross(v - branch.origin, branch.direction));
                    double w =
                            branch.radiusBegin * (1 - dotA / branch.length) + branch.radiusEnd * dotA / branch.length;
                    if (dist <= w) {
                        //grid->SetVoxel({i, j, k}, 0x4a301a);
                        grid->SetVoxel({i, j, k}, 0xFFFFFF);
                    }
                } else if (dotA < 0) {
                    if (glm::length(v - branch.origin) <= branch.radiusBegin) {
                        //grid->SetVoxel({i, j, k}, 0x4a301a);
                        grid->SetVoxel({i, j, k}, 0xFFFFFF);
                    }
                } else {
                    if (glm::length(v - branch.direction * branch.length - branch.origin) <= branch.radiusEnd) {
                        //grid->SetVoxel({i, j, k}, 0x4a301a);
                        grid->SetVoxel({i, j, k}, 0xFFFFFF);
                    }
                }
            }
        }
    }

    for (auto &child: branch.childBranches) {
        PlaceBranch(child, grid);
    }

}

TreeGen::Branch TreeGen::GenerateTreeGraphAny() {
    Branch trunk;
    trunk.origin = glm::dvec3(0, 0, 0);
    trunk.direction = glm::normalize(glm::dvec3(0, 1, 0) + randomVec3(rng) * 0.2);
    trunk.length = rng.get_double(40, 50);
    trunk.radiusBegin = rng.get_double(4, 6);
    trunk.radiusEnd = trunk.radiusBegin * 0.8;

    GenerateTreeGraphRecursive(trunk, trunk);

    return trunk;
}

void TreeGen::GenerateTreeGraphRecursive(TreeGen::Branch &root, TreeGen::Branch &branch, int depth) {
    if (branch.length < 9 || branch.radiusBegin < 2) {
        return;
    }

    double startAt = rng.get_double(branch.length * 0.3, branch.length * 0.7);
    if (depth == 0) {
        startAt = branch.length;
    }

    for (int iter = 0; iter < 22 && branch.childBranches.size() < 3; iter++) {
        Branch newBranch;
        newBranch.origin = branch.origin + branch.direction * startAt;
        auto rvec = randomVec3(rng);
        rvec -= glm::dot(branch.direction, rvec) * branch.direction;
        rvec = glm::normalize(rvec);
        newBranch.direction = glm::normalize(branch.direction + rvec * rng.get_double(0.9, 2.0));
        newBranch.length = rng.get_double(0.6 * branch.length, 0.8 * branch.length);
        newBranch.radiusBegin = branch.radiusEnd * rng.get_double(0.7, 1.0);
        newBranch.radiusEnd = newBranch.radiusBegin * rng.get_double(0.6, 0.8);

        auto &nearest = FindNearest(root, newBranch);

        if (glm::dot(glm::dvec3(0, 1, 0), newBranch.direction) < -0.4) continue;
        if (glm::length(nearest.getEnd() - newBranch.getEnd()) < newBranch.length * 0.7) continue;
        bool anyNear = false;
        for(auto & other : branch.childBranches) {
            if (glm::dot(other.direction, newBranch.direction) > 0.6) anyNear = true;
        }
        if (anyNear) continue;

        branch.childBranches.push_back(newBranch);
    }

    for (auto &child: branch.childBranches) {
        GenerateTreeGraphRecursive(root, child, depth + 1);
    }
}

TreeGen::Branch &TreeGen::FindNearest(TreeGen::Branch &root, TreeGen::Branch &branch) {
    Branch &res = root;
    std::function<void(Branch & )> findRecursive;
    findRecursive = [&](Branch &current) {
        if (glm::length(current.getEnd() - branch.getEnd()) < glm::length(res.getEnd() - branch.getEnd())) {
            res = current;
        }
        for (auto &child: current.childBranches) {
            findRecursive(child);
        }
    };
    return res;
}
