#include <utility>
#include <lit/engine/components/voxel_world.hpp>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <queue>

using namespace lit::engine;

namespace std {
    template<>
    struct hash<glm::ivec3> {
        std::size_t operator()(const glm::ivec3 &k) const {
            return std::hash<int>()(k.x)
                   ^ (std::hash<int>()(k.y) << 9)
                   ^ (std::hash<int>()(k.y) >> 3)
                   ^ (std::hash<int>()(k.z) << 7);
        }
    };
}

std::vector<glm::ivec3> DELTAS = {
        {-1, 0,  0},
        {0,  -1, 0},
        {0,  0,  -1},
        {+1, 0,  0},
        {0,  +1, 0},
        {0,  0,  +1},
};

VoxelWorld::VoxelWorld() {
    m_grid_data_with_lods.assign(GetGridWithLodsSize(), CHUNK_EMPTY);
    m_chunks.resize(1);
    m_positions.resize(1);
    m_chunk_index_allocator.Allocate();
}

void VoxelWorld::SetGenerator(std::function<void(glm::ivec3, ChunkRaw &)> chunk_generator) {
    m_chunk_generator = std::move(chunk_generator);
    m_chunk_index_allocator = FixedAllocator(1'000'000);
    m_grid_data_with_lods.assign(GetGridWithLodsSize(), CHUNK_UNKNOWN);
    m_chunks.resize(1);
    m_positions.resize(1);
    m_chunk_index_allocator.Allocate();
    m_version = 0;
    Generate(glm::ivec3(GetChunkGridDims() - glm::ivec3(1)));
}

void VoxelWorld::Generate(glm::ivec3 start_grid_position) {
    if (!m_chunk_generator || IsKnownChunk(start_grid_position)) {
        return;
    }

    std::unordered_set<glm::ivec3> queue;
    queue.insert(start_grid_position);

#ifdef PARALLEL_GENERATION
    std::vector<std::thread> threads;
    std::mutex mutex;
    int running = 0;
    for (int thread = 0; thread < 16; thread++) {
        threads.push_back(std::thread([this, &queue, &mutex, &running]() {
            while (true) {
                glm::ivec3 grid_position;
                ChunkIndexType index;
                ChunkWithLods * chunk;
                {
                    std::lock_guard<std::mutex> guard(mutex);
                    if (queue.empty() && !running) break;
                    if (queue.empty()) {
                        continue;
                    }
                    grid_position = *queue.begin();
                    queue.erase(queue.begin());
                    if (IsKnownChunk(grid_position) || !IsValidChunk(grid_position)) {
                        continue;
                    }

                    index = m_chunk_index_allocator.Allocate();
                    if (index >= m_chunks.size()) {
                        m_chunks.emplace_back();
                        m_positions.emplace_back();
                    }
                    chunk = &m_chunks[index];
                    m_positions[index] = grid_position;
                    running++;
                }

                std::fill(chunk->begin(), chunk->end(), 0u);
                m_chunk_generator(grid_position, reinterpret_cast<ChunkRaw &>(*chunk));

                {
                    std::lock_guard<std::mutex> guard(mutex);
                    if (std::all_of(chunk->begin(), chunk->end(), [](VoxelType v) { return !v; })) {
                        // if chunk is empty
                        m_chunk_index_allocator.Free(index);
                        SetEmpty(grid_position, queue);
                    } else {
                        SetChunk(grid_position, index, queue);
                        if (m_chunk_created) {
                            m_chunk_created(index);
                        }
                    }
                    running--;
                }
            }
        }));
    }
    for (auto &thread: threads) thread.join();
#else
    while (!queue.empty()) {
        auto grid_position = *queue.begin();
        queue.erase(queue.begin());
        if (IsKnownChunk(grid_position) || !IsValidChunk(grid_position)) {
            continue;
        }

        ChunkIndexType index = m_chunk_index_allocator.Allocate();
        if (index >= m_chunks.size()) {
            m_chunks.emplace_back();
            m_positions.emplace_back();
        }

        m_positions[index] = grid_position;

        std::fill(m_chunks[index].begin(), m_chunks[index].end(), 0u);
        m_chunk_generator(grid_position, reinterpret_cast<ChunkRaw &>(m_chunks[index]));
        if (std::all_of(m_chunks[index].begin(), m_chunks[index].end(), [](VoxelType v) { return !v; })) {
            // if chunk is empty
            m_chunk_index_allocator.Free(index);
            SetEmpty(grid_position, queue);
        } else {
            SetChunk(grid_position, index, queue);
            if (m_chunk_created) {
                m_chunk_created(index);
            }
        }
    }
#endif

    UpdateGrid();
}

void VoxelWorld::SetChunk(glm::ivec3 grid_position, ChunkIndexType chunk_index, std::unordered_set<glm::ivec3> &queue) {
    auto &chunk = reinterpret_cast<ChunkRaw &>(m_chunks[chunk_index]);
    glm::ivec3 delta = glm::ivec3(-1, 0, 0);
    if (IsValidChunk(grid_position + delta) && !IsKnownChunk(grid_position + delta)) {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            for (int j = 0; j < CHUNK_SIZE; j++) {
                if (chunk[0][i][j] == 0) {
                    queue.insert(grid_position + delta);
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
                    queue.insert(grid_position + delta);
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
                    queue.insert(grid_position + delta);
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
                    queue.insert(grid_position + delta);
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
                    queue.insert(grid_position + delta);
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
                    queue.insert(grid_position + delta);
                    goto f6;
                }
            }
        }
        f6:;
    }

    m_grid_changes++;
    m_version++;
    m_grid_data_with_lods[GridPosToIndex(grid_position)] = chunk_index;
}

void VoxelWorld::SetEmpty(glm::ivec3 grid_position, std::unordered_set<glm::ivec3> &queue) {
    for (auto &delta: DELTAS) {
        if (IsValidChunk(grid_position + delta) && !IsKnownChunk(grid_position + delta)) {
            queue.insert(grid_position + delta);
        }
    }
    m_grid_changes++;
    m_version++;
    m_grid_data_with_lods[GridPosToIndex(grid_position)] = CHUNK_EMPTY;
}

void VoxelWorld::UpdateChunk(ChunkIndexType index) {
    glm::ivec3 size = GetChunkDims();
    std::fill(m_chunks[index].begin() + glm::compMul(GetChunkDims()), m_chunks[index].end(), 0u);
    for (int lod = 1; lod < CHUNK_LOD_NUM; lod++) {
        for (int i = 0; i < size.x; i++) {
            for (int j = 0; j < size.y; j++) {
                for (int k = 0; k < size.z; k++) {
                    m_chunks[index][ChunkPosToIndex(glm::ivec3(i, j, k) >> 1, lod)] =
                            std::max(m_chunks[index][ChunkPosToIndex(glm::ivec3(i, j, k) >> 1, lod)], m_chunks[index][ChunkPosToIndex(glm::ivec3(i, j, k), lod - 1)]);
                }
            }
        }
        size >>= 1;
    }
}

void VoxelWorld::UpdateGrid() {
    if (!m_grid_changes) {
        return;
    }

    glm::ivec3 size = GetChunkGridDims();
    std::fill(m_grid_data_with_lods.begin() + glm::compMul(GetChunkGridDims()), m_grid_data_with_lods.end(), 0u);
    for (int lod = 1; lod < GRID_LOD_NUM; lod++) {
        for (int i = 0; i < size.x; i++) {
            for (int j = 0; j < size.y; j++) {
                for (int k = 0; k < size.z; k++) {
                    if (m_grid_data_with_lods[GridPosToIndex(glm::ivec3(i, j, k), lod - 1)] != CHUNK_UNKNOWN) {
                        m_grid_data_with_lods[GridPosToIndex(glm::ivec3(i, j, k) >> 1, lod)]
                                |= m_grid_data_with_lods[GridPosToIndex(glm::ivec3(i, j, k), lod - 1)];
                    }
                }
            }
        }
        size >>= 1;
    }

    if (m_grid_changes) {
        m_grid_changes = 0;
    }
}

bool VoxelWorld::IsValidChunk(glm::ivec3 grid_position) const {
    return glm::all(glm::greaterThanEqual(grid_position, glm::ivec3(0))) &&
           glm::all(glm::lessThan(grid_position, GetChunkGridDims()));
}

uint64_t VoxelWorld::GetVersion() const {
    return m_version;
}

void VoxelWorld::SetVoxel(glm::ivec3 pos, VoxelType value) {
    if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= GetDims().x || pos.y >= GetDims().y || pos.z >= GetDims().z) {
        return;
    }

    auto grid_position = pos >> CHUNK_SIZE_LOG;
    size_t grid_index = GridPosToIndex(grid_position);

    // Generate chunk if it is UNKNOWN.
    if (m_grid_data_with_lods[grid_index] == CHUNK_UNKNOWN) {
        Generate(grid_position);
    }

    // Do nothing if trying to set empty voxel in empty chunk.
    if (value == 0u && m_grid_data_with_lods[grid_index] == CHUNK_EMPTY) {
        return;
    }

    // If chunk is empty we should create a new one and set it.
    if (m_grid_data_with_lods[grid_index] == CHUNK_EMPTY) {
        ChunkIndexType index = m_chunk_index_allocator.Allocate();
        if (index >= m_chunks.size()) {
            m_chunks.emplace_back();
        }
        std::fill(m_chunks[index].begin(), m_chunks[index].end(), 0u);
        if (m_chunk_created) {
            m_chunk_created(grid_index);
        }
        std::unordered_set<glm::ivec3> q;
        SetChunk(grid_position, index, q);
    }

    auto pos_inside_chunk = pos & (CHUNK_SIZE - 1);
    size_t index_inside_chunk = ChunkPosToIndex(pos_inside_chunk);

    if (m_chunks[m_grid_data_with_lods[grid_index]][index_inside_chunk] != value) {
        if (m_chunk_changed) {
            m_chunk_changed(grid_index);
        }

        m_chunks[m_grid_data_with_lods[grid_index]][index_inside_chunk] = value;

        // if we are setting some non-empty voxel to empty we should check if we need to re-generate adjacent chunks.
        if (value == 0 && (glm::any(glm::equal(pos_inside_chunk, glm::ivec3())) ||
                           glm::any(glm::equal(pos_inside_chunk, glm::ivec3(CHUNK_SIZE - 1))))) {
            for (auto &delta: DELTAS) {
                glm::ivec3 position = (pos + delta) >> CHUNK_SIZE_LOG;
                if (!IsKnownChunk(position)) {
                    Generate(position);
                }
            }
        }
    }
}

VoxelWorld::VoxelType VoxelWorld::GetVoxel(glm::ivec3 pos) const {
    if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= GetDims().x || pos.y >= GetDims().y || pos.z >= GetDims().z) {
        return 0u;
    }

    auto grid_position = pos >> CHUNK_SIZE_LOG;
    size_t grid_index = GridPosToIndex(grid_position);

    if (m_grid_data_with_lods[grid_index] == CHUNK_EMPTY || m_grid_data_with_lods[grid_index] == CHUNK_UNKNOWN) {
        return 0u;
    }

    auto pos_inside_chunk = pos & (CHUNK_SIZE - 1);
    size_t index_inside_chunk = ChunkPosToIndex(pos_inside_chunk);

    return m_chunks[m_grid_data_with_lods[grid_index]][index_inside_chunk];
}

size_t VoxelWorld::GridPosToIndex(glm::ivec3 grid_position, int lod) const {
    size_t res = 0;
    auto size = GetChunkGridDims();
    for (int i = 0; i < lod; i++) {
        res += glm::compMul(size >> i);
    }
    size >>= lod;
    res += grid_position.x * (size.y * size.z) + grid_position.y * size.z + grid_position.z;
    return res;
}

size_t VoxelWorld::ChunkPosToIndex(glm::ivec3 position, int lod) const {
    size_t res = 0;
    auto size = GetChunkDims();
    for (int i = 0; i < lod; i++) {
        res += glm::compMul(size >> i);
    }
    size >>= lod;
    res += position.x * (size.y * size.z) + position.y * size.z + position.z;
    return res;
}

bool VoxelWorld::IsKnownChunk(glm::ivec3 grid_position) const {
    return m_grid_data_with_lods[GridPosToIndex(grid_position)] != CHUNK_UNKNOWN;
}

void VoxelWorld::WriteGridDataTo(VoxelWorld::ChunkIndexType *dest) {
    UpdateGrid();
    std::copy(m_grid_data_with_lods.begin(), m_grid_data_with_lods.end(), dest);
}

void VoxelWorld::WriteChunkDataTo(VoxelWorld::VoxelType *dest, VoxelWorld::ChunkIndexType index, int min_lod) {
    UpdateChunk(index);
    size_t offset = 0x49249249u & ~(~0u << (3 * CHUNK_SIZE_LOG + 1)) & (~0u << (3 * (CHUNK_SIZE_LOG - min_lod) + 1));
    std::copy(m_chunks[index].begin() + offset, m_chunks[index].end(), dest);
}

glm::ivec3 VoxelWorld::GetChunkCenterByIndex(VoxelWorld::ChunkIndexType index) const {
    return m_positions[index] * CHUNK_SIZE + glm::ivec3(CHUNK_SIZE >> 1);
}

void VoxelWorld::SetChunkDeletedCallback(std::function<void(ChunkIndexType)> callback) const {
    m_chunk_deleted = std::move(callback);
}

void VoxelWorld::SetChunkChangedCallback(std::function<void(ChunkIndexType)> callback) const {
    m_chunk_changed = std::move(callback);
}

void VoxelWorld::SetChunkCreatedCallback(std::function<void(ChunkIndexType)> callback) const {
    m_chunk_created = std::move(callback);
    for (int i = 0; i < GetChunkGridDims().x; i++) {
        for (int j = 0; j < GetChunkGridDims().y; j++) {
            for (int k = 0; k < GetChunkGridDims().z; k++) {
                auto index = m_grid_data_with_lods[GridPosToIndex(glm::ivec3(i, j, k))];
                if (index != CHUNK_UNKNOWN && index != CHUNK_EMPTY) {
                    m_chunk_created(index);
                }
            }
        }
    }
}

VoxelWorld::ChunkIndexType VoxelWorld::GetChunksNum() const {
    return m_chunks.size();
}

size_t VoxelWorld::GetSize() const {
    size_t res = 0;
    res += sizeof(VoxelWorld);
    res += m_chunks.size() * sizeof(ChunkWithLods);
    res += m_grid_data_with_lods.capacity() * sizeof(ChunkIndexType);
    res += m_positions.capacity() * sizeof(glm::ivec3);
    spdlog::default_logger()->trace("chunks: {} * {} = {}", m_chunks.size(), sizeof(ChunkWithLods),
                                    m_chunks.size() * sizeof(ChunkWithLods));
    spdlog::default_logger()->trace("grid: {} * {} = {}", m_grid_data_with_lods.capacity(), sizeof(ChunkIndexType),
                                    m_grid_data_with_lods.capacity() * sizeof(ChunkIndexType));
    spdlog::default_logger()->trace("positions: {} * {} = {}", m_positions.capacity(), sizeof(glm::ivec3),
                                    m_positions.capacity() * sizeof(glm::ivec3));
    return res;
}
