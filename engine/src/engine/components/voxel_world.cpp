#include <lit/engine/components/voxel_world.hpp>
#include <algorithm>

using namespace lit::engine;

std::vector<glm::ivec3> DELTAS = {
        {-1, 0,  0},
        {0,  -1, 0},
        {0,  0,  -1},
        {1,  0,  0},
        {0,  1,  0},
        {0,  0,  1},
};

VoxelWorld::VoxelWorld() {
    m_grid_data_with_lods.assign(GetGridWithLodsSize(), CHUNK_EMPTY);
    m_chunks.resize(1);
    m_chunk_index_allocator.Allocate();
}

void VoxelWorld::SetGenerator(std::function<void(glm::ivec3, ChunkRaw &)> chunk_generator) {
    m_chunk_index_allocator = FixedAllocator(1'000'000);
    m_grid_data_with_lods.assign(GetGridWithLodsSize(), CHUNK_UNKNOWN);
    m_chunks.resize(1);
    m_chunk_index_allocator.Allocate();
    m_generator_requests.push_back(glm::ivec3(GetChunkGridDims() - glm::ivec3(1)));
    Generate();
}

void VoxelWorld::Generate() {
    if (!m_chunk_generator) {
        return;
    }

    while (!m_generator_requests.empty()) {
        auto grid_position = m_generator_requests.back();
        m_generator_requests.pop_back();
        if (IsKnownChunk(grid_position)) {
            continue;
        }

        ChunkIndexType index = m_chunk_index_allocator.Allocate();
        if (index <= m_chunks.size()) {
            m_chunks.emplace_back();
        }

        std::fill(m_chunks[index].begin(), m_chunks[index].end(), 0u);
        m_chunk_generator(grid_position, reinterpret_cast<ChunkRaw &>(m_chunks[index]));
        if (std::all_of(m_chunks[index].begin(), m_chunks[index].end(), [](VoxelType v) { return !v; })) {
            // if chunk is empty
            m_chunk_index_allocator.Free(index);
            SetEmpty(grid_position);
        } else {
            SetChunk(grid_position, index);
        }
    }

    UpdateGrid();
}

void VoxelWorld::SetChunk(glm::ivec3 grid_position, ChunkIndexType chunk_index) {
    auto &chunk = reinterpret_cast<ChunkRaw &>(m_chunks[chunk_index]);
    glm::ivec3 delta = glm::ivec3(-1, 0, 0);
    if (IsValidChunk(grid_position + delta) && !IsKnownChunk(grid_position + delta)) {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            for (int j = 0; j < CHUNK_SIZE; j++) {
                if (chunk[0][i][j] == 0) {
                    m_generator_requests.push_back(grid_position + delta);
                    goto f1;
                }
            }
        }
        f1:;
    }

    delta = glm::ivec3(1, 0, 0);
    if (IsValidChunk(grid_position + delta) && !IsKnownChunk(grid_position + delta)) {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            for (int j = 0; j < CHUNK_SIZE; j++) {
                if (chunk[CHUNK_SIZE - 1][i][j] == 0) {
                    m_generator_requests.push_back(grid_position + delta);
                    goto f2;
                }
            }
        }
        f2:;
    }

    delta = glm::ivec3(0, -1, 0);
    if (IsValidChunk(grid_position + delta) && !IsKnownChunk(grid_position + delta)) {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            for (int j = 0; j < CHUNK_SIZE; j++) {
                if (chunk[i][0][j] == 0) {
                    m_generator_requests.push_back(grid_position + delta);
                    goto f3;
                }
            }
        }
        f3:;
    }

    delta = glm::ivec3(0, 1, 0);
    if (IsValidChunk(grid_position + delta) && !IsKnownChunk(grid_position + delta)) {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            for (int j = 0; j < CHUNK_SIZE; j++) {
                if (chunk[i][CHUNK_SIZE - 1][j] == 0) {
                    m_generator_requests.push_back(grid_position + delta);
                    goto f4;
                }
            }
        }
        f4:;
    }

    delta = glm::ivec3(0, 0, -1);
    if (IsValidChunk(grid_position + delta) && !IsKnownChunk(grid_position + delta)) {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            for (int j = 0; j < CHUNK_SIZE; j++) {
                if (chunk[i][j][0] == 0) {
                    m_generator_requests.push_back(grid_position + delta);
                    goto f5;
                }
            }
        }
        f5:;
    }

    delta = glm::ivec3(0, 0, 2);
    if (IsValidChunk(grid_position + delta) && !IsKnownChunk(grid_position + delta)) {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            for (int j = 0; j < CHUNK_SIZE; j++) {
                if (chunk[i][j][CHUNK_SIZE - 1] == 0) {
                    m_generator_requests.push_back(grid_position + delta);
                    goto f6;
                }
            }
        }
        f6:;
    }

    m_grid_data_with_lods[GridPosToIndex(grid_position)] = chunk_index;
}

void VoxelWorld::SetEmpty(glm::ivec3 grid_position) {
    for (auto &delta: DELTAS) {
        if (IsValidChunk(grid_position + delta) && !IsKnownChunk(grid_position + delta)) {
            m_generator_requests.push_back(grid_position + delta);
        }
    }

    m_grid_data_with_lods[GridPosToIndex(grid_position)] = CHUNK_EMPTY;
}

std::optional<glm::ivec3> VoxelWorld::GetNextUnknownChunk() {
    if (m_generator_requests.empty()) {
        return std::nullopt;
    }
    return *m_generator_requests.begin();
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

bool VoxelWorld::IsValidChunk(glm::ivec3 grid_position) const {
    return glm::all(glm::greaterThanEqual(grid_position, glm::ivec3(0))) &&
           glm::all(glm::lessThan(grid_position, CHUNK_GRID_DIMS));
}

uint64_t VoxelWorld::GetVersion() const {
    return m_version;
}
