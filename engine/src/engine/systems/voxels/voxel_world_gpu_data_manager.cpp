#include <lit/engine/systems/voxels/voxel_world_gpu_data_manager.hpp>
#include <thread>
#include <spdlog/spdlog.h>
#include <lit/viewer/debug_options.hpp>

using namespace lit::engine;

VoxelWorldGpuDataManager::VoxelWorldGpuDataManager() :
        m_world_data_buffer(UniformBuffer::Create({.size=WORLD_BUFFER_SIZE_BYTES})),
        m_chunk_data_buffer(UniformBuffer::Create({.size=CHUNK_BUFFER_SIZE_BYTES})),
        m_chunk_info_buffer(UniformBuffer::Create({.size=INFO_BUFFER_SIZE_BYTES})) {
    for (uint32_t bucket = 0; bucket < BUCKET_NUM; bucket++) {
        m_allocator[bucket] =
                FixedAllocator(BUCKET_SIZE_BYTES[bucket] / (GetChunkSizeDword(bucket) * sizeof(uint32_t)));
    }
}

void VoxelWorldGpuDataManager::Update(VoxelWorld &world, glm::dvec3 position) {
    if (!m_world_registered) {
        m_world_registered = true;

        world.WriteGridDataTo((VoxelWorld::ChunkIndexType *) m_world_data_buffer.GetHostPtr());

        m_prev_world_version = world.GetVersion();

        world.SetChunkCreatedCallback([&](VoxelWorld::ChunkIndexType index) {
            m_sorted_chunk_indices.push_back(index);
            while (m_chunk_address.size() <= index) {
                m_chunk_bucket.emplace_back(BUCKET_NUM - 1);
                m_chunk_address.emplace_back();
            }

            m_chunk_address[index] = m_allocator[m_chunk_bucket[index]].Allocate();

            uint32_t global_address = GetBucketOffsetDword(m_chunk_bucket[index]) +
                                      GetChunkSizeDword(m_chunk_bucket[index]) * m_chunk_address[index];

            world.WriteChunkDataTo(((VoxelWorld::VoxelType *) m_chunk_data_buffer.GetHostPtr()) + global_address,
                                   index, m_chunk_bucket[index]);
            ((ChunkInfo *) m_chunk_info_buffer.GetHostPtr())[index] = ChunkInfo{global_address, m_chunk_bucket[index]};
        });

        world.SetChunkChangedCallback([&](VoxelWorld::ChunkIndexType index) {
            uint32_t global_address = GetBucketOffsetDword(m_chunk_bucket[index]) +
                                      GetChunkSizeDword(m_chunk_bucket[index]) * m_chunk_address[index];
            world.WriteChunkDataTo(((VoxelWorld::VoxelType *) m_chunk_data_buffer.GetHostPtr()) + global_address,
                                   m_chunk_bucket[index]);
        });

        world.SetChunkDeletedCallback([&](VoxelWorld::ChunkIndexType index) {
            m_sorted_chunk_indices.erase(
                    std::find(m_sorted_chunk_indices.begin(), m_sorted_chunk_indices.end(), index));
        });

    } else {
        // todo: update only region (?)
        if (m_prev_world_version != world.GetVersion()) {
            m_prev_world_version = world.GetVersion();

            world.WriteGridDataTo((VoxelWorld::ChunkIndexType *) m_world_data_buffer.GetHostPtr());
        }
    }

    if (m_sorted_chunk_indices.empty())
        return;

    std::vector<double> distance(m_chunk_bucket.size());
    for (uint32_t i = 0; i < m_chunk_bucket.size(); i++) {
        distance[i] = glm::length(glm::dvec3(glm::dvec3(world.GetChunkCenterByIndex(i)) - position));
    }

    for (uint32_t step = 128; step > 1; step >>= 1) {
        for (uint32_t i = m_sorted_chunk_indices.size() - 1; i >= step; i--) {
            if (distance[m_sorted_chunk_indices[i]] < distance[m_sorted_chunk_indices[i - step]]) {
                std::swap(m_sorted_chunk_indices[i], m_sorted_chunk_indices[i - step]);
            }
        }
    }

    for (int stage = 0; stage < 2; stage++) {

        if (m_current_i >= m_sorted_chunk_indices.size() || true) {
            m_current_i = 0;
            m_current_bucket = 0;
            m_chunks_in_current_bucket = 0;
        }

        for (uint32_t updates = 0;
             m_current_i < m_sorted_chunk_indices.size() && updates < UPDATES_PER_FRAME; m_current_i++) {
            uint32_t index = m_sorted_chunk_indices[m_current_i];
            uint32_t old_bucket = m_chunk_bucket[index];

            if (m_current_bucket + 1 < BUCKET_NUM &&
                (m_chunks_in_current_bucket > m_allocator[m_current_bucket].GetSize() * 0.8)) {
                m_current_bucket++;
                m_chunks_in_current_bucket = 0;
            }

            if (old_bucket == m_current_bucket) {
                m_chunks_in_current_bucket++;
                continue;
            }

            if (m_current_bucket + 1 < BUCKET_NUM &&
                (m_chunks_in_current_bucket > m_allocator[m_current_bucket].GetSize() * 0.8 ||
                 !m_allocator[m_current_bucket].CanAllocate())) {
                m_current_bucket++;
                m_chunks_in_current_bucket = 0;
            }

            if (stage == 0 && old_bucket < m_current_bucket) {
                continue;
            }

            if (stage == 1 && old_bucket > m_current_bucket) {
                continue;
            }

            updates++;

            m_chunk_bucket[index] = m_current_bucket;
            m_allocator[old_bucket].Free(m_chunk_address[index]);
            m_chunk_address[index] = m_allocator[m_current_bucket].Allocate();

            uint32_t global_address = GetBucketOffsetDword(m_chunk_bucket[index]) +
                                      GetChunkSizeDword(m_chunk_bucket[index]) * m_chunk_address[index];

            world.WriteChunkDataTo(((VoxelWorld::VoxelType *) m_chunk_data_buffer.GetHostPtr()) + global_address,
                                   index, m_chunk_bucket[index]);
            ((ChunkInfo *) m_chunk_info_buffer.GetHostPtr())[index] = ChunkInfo{global_address, m_chunk_bucket[index]};

            m_chunks_in_current_bucket++;
        }
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
    uint64_t size = glm::compMul(VoxelWorld::GetChunkGridDims());
    for (int i = 0; i < lod; i++) {
        res += size;
        size >>= 3; // /=8
    }
    return res;
}

uint64_t VoxelWorldGpuDataManager::GetWorldLodSizeDword(int lod) const {
    return glm::compMul(VoxelWorld::GetChunkGridDims() >> lod);
}

uint64_t VoxelWorldGpuDataManager::GetChunkLodSizeDword(int lod) const {
    return (1 << ((VoxelWorld::CHUNK_SIZE_LOG - lod) * 3));
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
    for (int i = bucket; i <= VoxelWorld::CHUNK_SIZE_LOG; i++) {
        res += GetChunkLodSizeDword(i);
    }
    return res;
}

UniformBuffer &VoxelWorldGpuDataManager::GetChunkInfoBuffer() {
    return m_chunk_info_buffer;
}

uint64_t VoxelWorldGpuDataManager::GetBucketOffsetDword(uint32_t bucket) const {
    uint64_t res = 0;
    for (uint32_t i = 0; i < bucket; i++) {
        res += m_allocator[i].GetSize() * GetChunkSizeDword(i);
    }
    return res;
}
