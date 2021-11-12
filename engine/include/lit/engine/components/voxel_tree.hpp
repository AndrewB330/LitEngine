#pragma once

#include <glm/vec3.hpp>
#include <vector>
#include <array>

namespace lit::engine {

    class VoxelTree {
    public:
        explicit VoxelTree(glm::ivec3 dims);

        VoxelTree(glm::ivec3 dims, glm::dvec3 anchor);

        uint32_t GetVoxel(glm::ivec3 pos) const;

        uint32_t GetVoxel(int x, int y, int z) const;

        void SetVoxel(glm::ivec3 pos, uint32_t value);

        void SetVoxel(int x, int y, int z, uint32_t value);

        glm::ivec3 GetDims() const;

        glm::dvec3 GetAnchor() const;

        // todo: private
    public:
        struct TreeNode {
            std::array<uint32_t, 8> child;
        };

        glm::ivec3 m_dims;
        glm::dvec3 m_anchor;

        std::vector<uint32_t> m_id_pool{};
        std::vector<TreeNode> m_tree{};
    };

}