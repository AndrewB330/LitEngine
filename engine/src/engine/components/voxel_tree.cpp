#include <lit/engine/components/voxel_tree.hpp>

using namespace lit::engine;

VoxelTree::VoxelTree(glm::ivec3 dims, glm::dvec3 anchor):m_anchor(anchor) {
    int max_dim = std::max(dims.x, std::max(dims.y, dims.z));
    max_dim--;
    max_dim |= max_dim >> 1;
    max_dim |= max_dim >> 2;
    max_dim |= max_dim >> 4;
    max_dim |= max_dim >> 8;
    max_dim |= max_dim >> 16;
    max_dim++;
    max_dim = std::max(max_dim, 2);
    m_dims = glm::ivec3(max_dim);
    m_tree.resize(2);
}

lit::engine::VoxelTree::VoxelTree(glm::ivec3 dims): VoxelTree(dims, glm::dvec3(dims) / 2.0) {}

void VoxelTree::SetVoxel(int x, int y, int z, uint32_t value) {
    if (x < 0 || y < 0 || z < 0 || x >= m_dims.x || y >= m_dims.y || z >= m_dims.z) {
        return;
    }
    if (value) {
        value |= (1u << 31);
    }

    int level = m_dims.x;

    uint32_t cur_node = 1;

    std::vector<uint32_t> nodes;
    nodes.reserve(16);

    auto create_node = [&](uint32_t fill) {
        if (m_id_pool.empty()) {
            m_tree.emplace_back();
            std::fill(m_tree.back().child.begin(), m_tree.back().child.end(), fill);
            return (uint32_t) (m_tree.size() - 1);
        }
        uint32_t index = m_id_pool.back();
        m_tree[index] = TreeNode();
        m_id_pool.pop_back();
        std::fill(m_tree[index].child.begin(), m_tree[index].child.end(), fill);
        return index;
    };

    while (level > 1) {
        nodes.push_back(cur_node);

        level >>= 1;
        int bit = (x >= level) | ((y >= level) << 1) | ((z >= level) << 2);
        x &= (level - 1);
        y &= (level - 1);
        z &= (level - 1);

        if (level == 1) {
            m_tree[cur_node].child[bit] = value;
        } else if (m_tree[cur_node].child[bit] != value) {
            if ((m_tree[cur_node].child[bit] >> 31) || m_tree[cur_node].child[bit] == 0) {
                uint32_t new_node = create_node(m_tree[cur_node].child[bit]);
                m_tree[cur_node].child[bit] = new_node;
                cur_node = new_node;
            } else {
                cur_node = m_tree[cur_node].child[bit];
            }
        } else {
            return;
        }
    }

    while (!nodes.empty()) {
        uint32_t node = nodes.back();
        nodes.pop_back();

        for(int i = 0; i < 7; i++) {
            if (m_tree[node].child[i] != m_tree[node].child[i+1]) return;
        }

        if (!nodes.empty()) {
            // delete node
            m_id_pool.push_back(node);

            for (int i = 0; i < 8; i++) {
                if (m_tree[nodes.back()].child[i] == node) {
                    m_tree[nodes.back()].child[i] = m_tree[node].child[0];
                }
            }
        }
    }
}

void VoxelTree::SetVoxel(glm::ivec3 pos, uint32_t value) {
    SetVoxel(pos.x, pos.y, pos.z, value);
}

uint32_t VoxelTree::GetVoxel(glm::ivec3 pos) const {
    return GetVoxel(pos.x, pos.y, pos.z);
}

uint32_t VoxelTree::GetVoxel(int x, int y, int z) const {
    if (x < 0 || y < 0 || z < 0 || x >= m_dims.x || y >= m_dims.y || z >= m_dims.z) {
        return 0;
    }

    int level = m_dims.x;

    uint32_t cur_node = 1;
    while (level > 1) {
        level >>= 1;
        int bit = (x >= level) | ((y >= level) << 1) | ((z >= level) << 2);
        x &= (level - 1);
        y &= (level - 1);
        z &= (level - 1);
        if ((m_tree[cur_node].child[bit] >> 31) || m_tree[cur_node].child[bit] == 0) {
            return m_tree[cur_node].child[bit] & ((1u << 31) - 1);
        }
        cur_node = m_tree[cur_node].child[bit];
    }
    return 0;
}

glm::ivec3 VoxelTree::GetDims() const {
    return m_dims;
}

glm::dvec3 VoxelTree::GetAnchor() const {
    return m_anchor;
}

