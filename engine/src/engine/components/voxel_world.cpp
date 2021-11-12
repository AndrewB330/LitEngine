#include <lit/engine/components/voxel_world.hpp>

using namespace lit::engine;

VoxelChunk::VoxelChunk() {
    static uint64_t unique_id = 0;
    m_uid = std::make_unique<uint64_t>(++unique_id);

    for (int i = 0; i <= CHUNK_SIZE_LOG; i++) {
        m_data[i] = lit::common::Image3D<uint32_t>(CHUNK_SIZE >> i, CHUNK_SIZE >> i, CHUNK_SIZE >> i);
    }
}

void VoxelChunk::SetVoxel(int x, int y, int z, uint32_t value) {
    m_changes += m_data[0].GetPixel(x, y, z) != value;
    m_data[0].SetPixel(x, y, z, value);
}

void VoxelChunk::SetVoxel(glm::ivec3 pos, uint32_t value) {
    SetVoxel(pos.x, pos.y, pos.z, value);
}

uint32_t VoxelChunk::GetVoxel(int x, int y, int z) const {
    return m_data[0].GetPixel(x, y, z);
}

uint32_t VoxelChunk::GetVoxel(glm::ivec3 pos, uint32_t value) const {
    return m_data[0].GetPixel(pos);
}

void VoxelChunk::Update() {
    if (m_changes == 0) {
        return;
    }

    glm::ivec3 size = glm::ivec3(CHUNK_SIZE);
    for (int lod = 1; lod <= CHUNK_SIZE_LOG; lod++) {
        m_data[lod].Fill(0);
        for (int i = 0; i < size.x; i++)
            for (int j = 0; j < size.y; j++)
                for (int k = 0; k < size.z; k++)
                    m_data[lod].GetPixel(i >> 1, j >> 1, k >> 1) |= m_data[lod - 1].GetPixel(i, j, k);
        size >>= 1;
    }

    if (m_changes) {
        m_changes = 0;
        m_version++;
    }
}

const uint32_t *VoxelChunk::GetDataPointer(int lod) const {
    return m_data[lod].GetDataPointer();
}

uint64_t VoxelChunk::GetVersion() const {
    return m_version;
}

uint64_t VoxelChunk::GetUid() const {
    return *m_uid;
}

glm::ivec3 VoxelChunk::GetChunkPos() const {
    return m_position;
}

std::vector<glm::ivec3> DELTAS = {
        {-1, 0,  0},
        {0,  -1, 0},
        {0,  0,  -1},
        {1,  0,  0},
        {0,  1,  0},
        {0,  0,  1},
};

VoxelWorld::VoxelWorld() {
    auto size = CHUNK_GRID_DIMS;
    for (int i = 0; i < GRID_LOD_NUM; i++) {
        m_data[i] = lit::common::Image3D<uint32_t>(size.x, size.y, size.z, CHUNK_UNKNOWN);
        size >>= 1;
    }
    m_chunks.resize(1);
    m_requests.insert(glm::ivec3(CHUNK_GRID_DIMS) - glm::ivec3(1));
}

void VoxelWorld::SetChunk(glm::ivec3 chunk_pos, VoxelChunk && chunk) {
    auto it = m_requests.find(chunk_pos);
    if (it != m_requests.end()) {
        m_requests.erase(it);
    }

    glm::ivec3 delta = glm::ivec3(-1, 0, 0);
    if (IsValidChunk(chunk_pos + delta) && !IsKnownChunk(chunk_pos + delta)) {
        for (int i = 0; i < VoxelChunk::CHUNK_SIZE; i++) {
            for (int j = 0; j < VoxelChunk::CHUNK_SIZE; j++) {
                if (chunk.GetVoxel(0, i, j) == 0) {
                    m_requests.insert(chunk_pos + delta);
                    goto f1;
                }
            }
        }
        f1:;
    }

    delta = glm::ivec3(1, 0, 0);
    if (IsValidChunk(chunk_pos + delta) && !IsKnownChunk(chunk_pos + delta)) {
        for (int i = 0; i < VoxelChunk::CHUNK_SIZE; i++) {
            for (int j = 0; j < VoxelChunk::CHUNK_SIZE; j++) {
                if (chunk.GetVoxel(VoxelChunk::CHUNK_SIZE - 1, i, j) == 0) {
                    m_requests.insert(chunk_pos + delta);
                    goto f2;
                }
            }
        }
        f2:;
    }

    delta = glm::ivec3(0, -1, 0);
    if (IsValidChunk(chunk_pos + delta) && !IsKnownChunk(chunk_pos + delta)) {
        for (int i = 0; i < VoxelChunk::CHUNK_SIZE; i++) {
            for (int j = 0; j < VoxelChunk::CHUNK_SIZE; j++) {
                if (chunk.GetVoxel(i, 0, j) == 0) {
                    m_requests.insert(chunk_pos + delta);
                    goto f3;
                }
            }
        }
        f3:;
    }

    delta = glm::ivec3(0, 1, 0);
    if (IsValidChunk(chunk_pos + delta) && !IsKnownChunk(chunk_pos + delta)) {
        for (int i = 0; i < VoxelChunk::CHUNK_SIZE; i++) {
            for (int j = 0; j < VoxelChunk::CHUNK_SIZE; j++) {
                if (chunk.GetVoxel(i, VoxelChunk::CHUNK_SIZE - 1, j) == 0) {
                    m_requests.insert(chunk_pos + delta);
                    goto f4;
                }
            }
        }
        f4:;
    }

    delta = glm::ivec3(0, 0, -1);
    if (IsValidChunk(chunk_pos + delta) && !IsKnownChunk(chunk_pos + delta)) {
        for (int i = 0; i < VoxelChunk::CHUNK_SIZE; i++) {
            for (int j = 0; j < VoxelChunk::CHUNK_SIZE; j++) {
                if (chunk.GetVoxel(i, j, 0) == 0) {
                    m_requests.insert(chunk_pos + delta);
                    goto f5;
                }
            }
        }
        f5:;
    }

    delta = glm::ivec3(0, 0, 2);
    if (IsValidChunk(chunk_pos + delta) && !IsKnownChunk(chunk_pos + delta)) {
        for (int i = 0; i < VoxelChunk::CHUNK_SIZE; i++) {
            for (int j = 0; j < VoxelChunk::CHUNK_SIZE; j++) {
                if (chunk.GetVoxel(i, j, VoxelChunk::CHUNK_SIZE - 1) == 0) {
                    m_requests.insert(chunk_pos + delta);
                    goto f6;
                }
            }
        }
        f6:;
    }
    chunk.m_position = chunk_pos;
    uint32_t index = 0;
    if (m_index_pool.empty()) {
        index = m_chunks.size();
        m_chunks.push_back(std::move(chunk));
    } else {
        index = m_index_pool.back();
        m_index_pool.pop_back();
        m_chunks[index] = std::move(chunk);
    }
    m_changes += m_data[0].GetPixel(chunk_pos) != index;
    m_data[0].SetPixel(chunk_pos, index);
}

void VoxelWorld::SetEmpty(glm::ivec3 chunk_pos) {
    auto it = m_requests.find(chunk_pos);
    if (it != m_requests.end()) {
        m_requests.erase(it);
    }

    auto &val = m_data[0].GetPixel(chunk_pos);
    if (val != 0 && val != CHUNK_UNKNOWN) {
        m_index_pool.push_back(val);
    }
    val = 0;
    for (auto &delta: DELTAS) {
        if (IsValidChunk(chunk_pos + delta) && !IsKnownChunk(chunk_pos + delta)) {
            m_requests.insert(chunk_pos + delta);
        }
    }
}

std::optional<glm::ivec3> VoxelWorld::GetNextUnknownChunk() {
    if (m_requests.empty()) {
        return std::nullopt;
    }
    return *m_requests.begin();
}

bool VoxelWorld::IsKnownChunk(glm::ivec3 chunk_pos) const {
    return m_data[0].GetPixel(chunk_pos) != CHUNK_UNKNOWN;
}

void VoxelWorld::Update() {
    if (!m_changes) {
        return;
    }

    glm::ivec3 size = CHUNK_GRID_DIMS;
    for (int lod = 1; lod < GRID_LOD_NUM; lod++) {
        m_data[lod].Fill(0);
        for (int i = 0; i < size.x; i++)
            for (int j = 0; j < size.y; j++)
                for (int k = 0; k < size.z; k++) {
                    if (m_data[lod - 1].GetPixel(i, j, k) != CHUNK_UNKNOWN) {
                        m_data[lod].GetPixel(i >> 1, j >> 1, k >> 1) |= m_data[lod - 1].GetPixel(i, j, k);
                    }
                }
        size >>= 1;
    }

    if (m_changes) {
        m_changes = 0;
        m_version++;
    }
}

bool VoxelWorld::IsValidChunk(glm::ivec3 chunk_pos) const {
    return glm::all(glm::greaterThanEqual(chunk_pos, glm::ivec3(0))) && glm::all(glm::lessThan(chunk_pos, CHUNK_GRID_DIMS));
}

const uint32_t *VoxelWorld::GetDataPointer(int lod) const {
    return m_data[lod].GetDataPointer();
}

const std::vector<VoxelChunk> &VoxelWorld::GetChunks() const {
    return m_chunks;
}

uint64_t VoxelWorld::GetVersion() const {
    return m_version;
}
