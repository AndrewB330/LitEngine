#include <lit/engine/systems/voxels/voxel_world_gpu_data_manager.hpp>
#include <thread>
#include <spdlog/spdlog.h>
#include <lit/viewer/debug_options.hpp>

using namespace lit::engine;

VoxelWorldGpuDataManager::VoxelWorldGpuDataManager() :
        m_world_data_buffer(UniformBuffer::Create(WORLD_BUFFER_SIZE_BYTES)),
        m_chunk_data_buffer(UniformBuffer::Create(CHUNK_BUFFER_SIZE_BYTES)),
        m_chunk_info_buffer(UniformBuffer::Create(INFO_BUFFER_SIZE_BYTES)) {
    for (uint32_t bucket = 0; bucket < BUCKET_NUM; bucket++) {
        m_allocator[bucket] =
                FixedAllocator(BUCKET_SIZE_BYTES[bucket] / (GetChunkSizeDword(bucket) * sizeof(uint32_t)));
    }
}

void VoxelWorldGpuDataManager::Update(const VoxelWorld &world, glm::dvec3 position) {
    if (!DebugOptions::Instance().update_chunks) {
        return;
    }

    if (!m_world_registered) {
        m_world_registered = true;

        RefreshWorldData(world);

        m_prev_world_version = world.GetVersion();
    } else {
        // todo: update only region (?)
        if (m_prev_world_version != world.GetVersion()) {
            m_prev_world_version = world.GetVersion();

            RefreshWorldData(world);
        }
    }

    auto &chunks = world.GetChunks();

    std::vector<uint32_t> chunks_updated;
    std::set<uint32_t> chunks_removed;
    std::vector<uint32_t> chunks_added;

    for (uint32_t i = 1; i < chunks.size(); i++) {
        uint64_t uid = chunks[i].GetUid();
        if (m_chunk_uids.find(uid) == m_chunk_uids.end()) {
            chunks_added.push_back(i);
            chunks_updated.push_back(i);
            if (i < m_prev_chunk_versions.size()) {
                chunks_removed.insert(i);
            }
        } else if (m_prev_chunk_versions.at(i) != chunks[i].GetVersion()) {
            chunks_updated.push_back(i);
        }
    }

    for (uint32_t index: chunks_updated) {
        while (m_prev_chunk_versions.size() <= index) {
            m_prev_chunk_versions.emplace_back();
            m_chunk_bucket.emplace_back();
            m_chunk_address.emplace_back(0xFFFFFFFF);
        }

        if (index == 0) {
            continue;
        }

        auto &chunk = chunks.at(index);

        if (m_chunk_uids.find(chunk.GetUid()) == m_chunk_uids.end()) {
            if (m_chunk_address.at(index) != 0xFFFFFFFF) {
                m_allocator[m_chunk_bucket.at(index)].Free(m_chunk_address.at(index));
            }
            m_chunk_bucket.at(index) = BUCKET_NUM - 1;
            m_chunk_address.at(index) = m_allocator[m_chunk_bucket.at(index)].Allocate();
        }
        m_prev_chunk_versions.at(index) = chunk.GetVersion();
        m_chunk_uids.insert(chunk.GetUid());

        RefreshChunkData(chunk, index);
    }

    if (!DebugOptions::Instance().phase0) {
        return;
    }

    uint32_t last = 0;
    for (uint32_t i = 0; i < m_sorted_chunk_indices.size(); i++) {
        if (chunks_removed.find(m_sorted_chunk_indices[i]) == chunks_removed.end()) {
            m_sorted_chunk_indices[last++] = m_sorted_chunk_indices[i];
        }
    }

    m_sorted_chunk_indices.resize(last);

    for (auto index: chunks_added) {
        m_sorted_chunk_indices.push_back(index);
    }

    double max_dist = 0;
    std::vector<double> distance(chunks.size());
    for (uint32_t i = 0; i < chunks.size(); i++) {
        distance[i] = glm::length(glm::dvec3(
                chunks[i].GetChunkPos() * VoxelChunk::CHUNK_SIZE + glm::ivec3(VoxelChunk::CHUNK_SIZE >> 1)) - position);
        max_dist = std::max(max_dist, distance[i]);
    }

    std::vector<std::vector<uint32_t>> b(20);
    for(uint32_t i = 0; i < m_sorted_chunk_indices.size(); i++) {
        b[int(distance[m_sorted_chunk_indices[i]] / (max_dist / 18))].push_back(m_sorted_chunk_indices[i]);
    }

    m_sorted_chunk_indices.clear();
    for(int i = 0; i < 20; i++) {
        m_sorted_chunk_indices.insert(m_sorted_chunk_indices.end(), b[i].begin(), b[i].end());
    }

    if (!DebugOptions::Instance().phase1) {
        return;
    }

    uint32_t current_bucket = 0;
    uint32_t chunks_in_current_bucket = 0;

    for (uint32_t i = 0; i < m_sorted_chunk_indices.size(); i++) {
        uint32_t index = m_sorted_chunk_indices[i];
        uint32_t old_bucket = m_chunk_bucket[index];

        if (old_bucket == current_bucket) {
            chunks_in_current_bucket++;
            continue;
        }

        if (current_bucket + 1 < BUCKET_NUM &&
            (chunks_in_current_bucket > m_allocator[current_bucket].GetSize() * 0.8 ||
             !m_allocator[current_bucket].CanAllocate())) {
            current_bucket++;
            chunks_in_current_bucket = 0;
        }

        m_chunk_bucket[index] = current_bucket;
        m_allocator[old_bucket].Free(m_chunk_address[index]);
        m_chunk_address[index] = m_allocator[current_bucket].Allocate();

        RefreshChunkData(chunks[index], index);
        chunks_in_current_bucket++;
    }
}

UniformBuffer &VoxelWorldGpuDataManager::GetWorldDataBuffer() {
    return m_world_data_buffer;
}

UniformBuffer &VoxelWorldGpuDataManager::GetChunkDataBuffer() {
    return m_chunk_data_buffer;
}

uint64_t VoxelWorldGpuDataManager::GetWorldLodOffsetDword(int lod) const {
    uint64_t res = 0;
    uint64_t size = glm::compMul(VoxelWorld::CHUNK_GRID_DIMS);
    for (int i = 0; i < lod; i++) {
        res += size;
        size >>= 3; // /=8
    }
    return res;
}

uint64_t VoxelWorldGpuDataManager::GetWorldLodSizeDword(int lod) const {
    return glm::compMul(VoxelWorld::CHUNK_GRID_DIMS >> lod);
}

uint64_t VoxelWorldGpuDataManager::GetChunkLodSizeDword(int lod) const {
    return (1 << ((VoxelChunk::CHUNK_SIZE_LOG - lod) * 3));
}

uint64_t VoxelWorldGpuDataManager::GetChunkLodOffsetDword(int bucket, int lod) const {
    uint64_t res = 0;
    for (int i = bucket; i < lod; i++) {
        res += GetChunkLodSizeDword(i);
    }
    return res;
}

uint64_t VoxelWorldGpuDataManager::GetChunkSizeDword(int bucket) const {
    uint64_t res = 0;
    for (int i = bucket; i <= VoxelChunk::CHUNK_SIZE_LOG; i++) {
        res += GetChunkLodSizeDword(i);
    }
    return res;
}

UniformBuffer &VoxelWorldGpuDataManager::GetChunkInfoBuffer() {
    return m_chunk_info_buffer;
}

void VoxelWorldGpuDataManager::RefreshWorldData(const VoxelWorld &world) {
    auto *dest_ptr = (uint32_t *) m_world_data_buffer.GetHostPtr();
    for (int lod = 0; lod < VoxelWorld::GRID_LOD_NUM; lod++) {
        const uint32_t *src_ptr = world.GetDataPointer(lod);
        std::copy(src_ptr, src_ptr + GetWorldLodSizeDword(lod), dest_ptr + GetWorldLodOffsetDword(lod));
    }
}

void VoxelWorldGpuDataManager::RefreshChunkData(const VoxelChunk &chunk, uint32_t index) {
    uint32_t bucket = m_chunk_bucket[index];
    uint32_t global_address = GetBucketOffsetDword(bucket) + m_chunk_address[index] * GetChunkSizeDword(bucket);
    auto *dest_ptr = (uint32_t *) m_chunk_data_buffer.GetHostPtr() + global_address;
    auto *info_dest_ptr = (ChunkInfo *) m_chunk_info_buffer.GetHostPtr();
    for (uint32_t lod = bucket; lod < VoxelWorld::GRID_LOD_NUM; lod++) {
        const uint32_t *src_ptr = chunk.GetDataPointer(lod);
        std::copy(src_ptr, src_ptr + GetChunkLodSizeDword(lod), dest_ptr + GetChunkLodOffsetDword(bucket, lod));
    }
    info_dest_ptr[index] = ChunkInfo{global_address, bucket};
}

uint64_t VoxelWorldGpuDataManager::GetBucketOffsetDword(uint32_t bucket) const {
    uint64_t res = 0;
    for (uint32_t i = 0; i < bucket; i++) {
        res += m_allocator[i].GetSize() * GetChunkSizeDword(i);
    }
    return res;
}
