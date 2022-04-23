#pragma once

#include <lit/common/random.hpp>
#include <lit/engine/components/voxel_grid/voxel_grid.hpp>
#include <lit/common/random.hpp>
#include <spdlog/spdlog.h>

namespace lit::engine {
    class TreeGen {
    public:
        TreeGen(int seed = 0);

        VoxelGridPtr GenerateTreeAny();

    private:

        struct Branch {
            glm::dvec3 origin;
            glm::dvec3 direction;
            double length;
            double radiusBegin;
            double radiusEnd;

            std::vector<Branch> childBranches;

            glm::dvec3 getEnd() const {
                return origin + direction * length;
            }
        };

        Branch GenerateTreeGraphAny();

        void GenerateTreeGraphRecursive(Branch &root, Branch &branch, int depth = 0);

        Branch &FindNearest(Branch &root, Branch &branch);

        void PlaceBranch(const Branch &branch, const VoxelGridPtr &grid);

        RandomGen rng;
    };
}