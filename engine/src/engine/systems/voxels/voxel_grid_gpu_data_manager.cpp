#include <lit/engine/systems/voxels/voxel_grid_gpu_data_manager.hpp>
#include <thread>
#include <spdlog/spdlog.h>
#include <lit/viewer/debug_options.hpp>
#include "lit/engine/components/transform.hpp"

using namespace lit::engine;

VoxelGridGpuDataManager::VoxelGridGpuDataManager(entt::registry &registry, VoxelGridLodManager<uint32_t> &lod_manager) :
        m_chunk_grid_data_buffer(UniformBuffer::Create({.size = CHUNK_GRID_BUFFER_SIZE_BYTES})),
        m_chunk_data_buffer(UniformBuffer::Create({.size = CHUNK_BUFFER_SIZE_BYTES})),
        m_chunk_bit_data_buffer(UniformBuffer::Create({.size = CHUNK_BIT_BUFFER_SIZE_BYTES})),
        m_chunk_info_buffer(UniformBuffer::Create({.size = INFO_BUFFER_SIZE_BYTES})),
        System(registry),
        m_lod_manager(lod_manager) {
    for (uint32_t bucket = 0; bucket < BUCKET_NUM; bucket++) {
        m_allocator[bucket] =
                ContiguousAllocator(BUCKET_SIZE_BYTES[bucket] / (GetChunkLodSizeDword(bucket) * sizeof(uint32_t)));
    }
}

void VoxelGridGpuDataManager::RegisterNewEntities() {
    for (auto ent: m_registry.view<VoxelGrid, VoxelGridLod>()) {
        auto &grid = m_registry.get<VoxelGrid>(ent);
        auto &grid_lod = m_registry.get<VoxelGridLod>(ent);

        if (m_registered) {
            return;
        }

        m_registered = true;

        grid.InvokeForAllChunks([ent, this](const VoxelGrid::ChunkView &v) {
            m_changes[ent].push_back(typename VoxelGrid::ChunkCreatedArgs{v.GetIndex(), v.GetChunkGridPosition()});
        });

        size_t handle = grid.AddOnChunkAnyChangeCallback([ent, this](ChunkAnyChangeArgs args) {
            m_changes[ent].push_back(args);
        });
    }
}

void lit::engine::VoxelGridGpuDataManager::ProcessAllChangesForEntity(entt::entity ent,
                                                                      const std::vector<ChunkAnyChangeArgs> &changes) {
    // Method that processes all chunk __changes__ to the main sparse grid.
    // Possible changes: Chunk created, chunk removed, chunk edited.

    auto &grid_lod = m_registry.get<VoxelGridLod>(ent);
    auto &grid = m_registry.get<VoxelGrid>(ent);

    if (changes.empty()) {
        return;
    }

    bool chunk_grid_updated = std::any_of(changes.begin(), changes.end(), [](ChunkAnyChangeArgs args) {
        return std::holds_alternative<ChunkCreatedArgs>(args) || std::holds_alternative<ChunkDeletedArgs>(args);
    });

    // just update chunk grid (indices of the chunks and info about empty chunks, or zero chunks)
    if (chunk_grid_updated) {
        memcpy(m_chunk_grid_data_buffer.GetHostPtr(), grid_lod.m_grid_lod_data.data(),
               sizeof(uint32_t) * grid_lod.m_grid_lod_data.size());
    }

    std::unordered_set<VoxelGrid::ChunkIndexType> chunks_to_update;

    typename VoxelGrid::ChunkIndexType max_index = 0;

    // Determine which chunks need to be updated.
    for (auto &change: changes) {
        if (std::holds_alternative<ChunkCreatedArgs>(change)) {
            auto index = std::get<ChunkCreatedArgs>(change).index;
            chunks_to_update.insert(index);
            max_index = std::max(max_index, index);
            m_sorted_chunk_indices.push_back(index);

            if (m_chunk_bucket.size() <= max_index) {
                m_chunk_bucket.resize(max_index + 1);
            }
            if (m_chunk_address.size() <= max_index) {
                m_chunk_address.resize(max_index + 1);
            }

            // Put new chunk to the least detailed bucket.
            // It will find the right bucket later.
            // TODO: handle error in case if there is no space to allocate new chunk
            // TODO: this should not really happen, but can happen if world is too sparse and big
            m_chunk_bucket.at(index) = BUCKET_NUM - 1;
            m_chunk_address.at(index) = m_allocator[m_chunk_bucket.at(index)].Allocate();
        } else if (std::holds_alternative<ChunkChangedArgs>(change)) {
            chunks_to_update.insert(std::get<ChunkChangedArgs>(change).index);
        } else if (std::holds_alternative<ChunkDeletedArgs>(change)) {
            auto index = std::get<ChunkDeletedArgs>(change).index;
            auto it = chunks_to_update.find(index);
            if (it != chunks_to_update.end()) {
                chunks_to_update.erase(it);
            }

            m_allocator[m_chunk_bucket.at(index)].Free(m_chunk_address.at(index));
            m_sorted_chunk_indices.erase(
                    std::find(m_sorted_chunk_indices.begin(), m_sorted_chunk_indices.end(), index));
        }
    }

    // Update bit-compressed lod data for changed chunks
    for (auto index: chunks_to_update) {
        size_t offset_elements_begin =
                index * ((GetLodTotalSize(VoxelGrid::GetChunkDimensions(), 0, VoxelGrid::CHUNK_SIZE_LOG) + 31) / 32);
        size_t size_bits = GetLodTotalSize(VoxelGrid::GetChunkDimensions(), 0, VoxelGrid::CHUNK_SIZE_LOG);
        size_t offset_elements_end = offset_elements_begin + (size_bits + 31) / 32;
        assert((0x49249249u & ~((~0u) << (3 * 5 + 1))) == size_bits);
        std::vector<uint32_t> a(grid_lod.m_chunk_binary_lod_data.data() + offset_elements_begin,
                                grid_lod.m_chunk_binary_lod_data.data() + offset_elements_end);

        memcpy((uint32_t *) m_chunk_bit_data_buffer.GetHostPtr() + offset_elements_begin,
               grid_lod.m_chunk_binary_lod_data.data() + offset_elements_begin,
               (offset_elements_end - offset_elements_begin) * sizeof(uint32_t));

        if (m_chunk_bucket.at(index) == 0) {
            memcpy((uint32_t *) m_chunk_data_buffer.GetHostPtr() + GetGlobalAddress(index),
                   grid.GetChunkViewAsArray(index).Data(),
                   (GetChunkLodSizeDword(m_chunk_bucket.at(index))) * sizeof(uint32_t));
        } else {
            memcpy((uint32_t *) m_chunk_data_buffer.GetHostPtr() + GetGlobalAddress(index),
                   grid_lod.GetChunkViewAtLod(index, m_chunk_bucket.at(index)).Data(),
                   (GetChunkLodSizeDword(m_chunk_bucket.at(index))) * sizeof(uint32_t));
        }

        ((ChunkInfo *) m_chunk_info_buffer.GetHostPtr())[index] = ChunkInfo{GetGlobalAddress(index),
                                                                            m_chunk_bucket.at(index)};
        //((ChunkInfo*)m_chunk_info_buffer.GetHostPtr())[index] = ChunkInfo{ 0, 0 };
        //((ChunkInfo*)m_chunk_info_buffer.GetHostPtr())[index] = ChunkInfo{ 0, m_chunk_bucket.at(index) };
    }

    /*if (!m_registered) {
        m_registered = true;

        grid.WriteGridDataTo((VoxelGridSparseT<uint32_t>::ChunkIndexType*) m_chunk_grid_data_buffer.GetHostPtr());

        m_prev_world_version = world.GetVersion();

        world.SetChunkCreatedCallback([&](VoxelGridSparseT<uint32_t>::ChunkIndexType index) {
            m_sorted_chunk_indices.push_back(index);
            while (m_chunk_address.size() <= index) {
                //TODO: !!! m_chunk_bucket.emplace_back(BUCKET_NUM - 1);
                m_chunk_bucket.emplace_back(0);
                m_chunk_address.emplace_back();
            }

            m_chunk_address.at(index) = m_allocator[m_chunk_bucket.at(index)].Allocate();

            uint32_t global_address = GetBucketOffsetDword(m_chunk_bucket.at(index)) +
                GetChunkLodSizeDword(m_chunk_bucket.at(index)) * m_chunk_address.at(index);

            uint32_t global_compress_address = ((GetChunkSizeDword(0) + 31) / 32) * index;

            world.WriteChunkDataTo(((VoxelGridSparseT<uint32_t>::VoxelType*)m_chunk_data_buffer.GetHostPtr()) + global_address,
                ((uint32_t*)m_chunk_compressed_data_buffer.GetHostPtr()) + global_compress_address,
                index, m_chunk_bucket.at(index));
            ((ChunkInfo*)m_chunk_info_buffer.GetHostPtr()).at(index) = ChunkInfo{ global_address, m_chunk_bucket.at(index) };
        });

        world.SetChunkChangedCallback([&](VoxelGridSparseT<uint32_t>::ChunkIndexType index) {
            uint32_t global_address = GetBucketOffsetDword(m_chunk_bucket.at(index)) +
                GetChunkLodSizeDword(m_chunk_bucket.at(index)) * m_chunk_address.at(index);
            uint32_t global_compress_address = ((GetChunkSizeDword(0) + 31) / 32) * index;
            world.WriteChunkDataTo(((VoxelGridSparseT<uint32_t>::VoxelType*)m_chunk_data_buffer.GetHostPtr()) + global_address,
                ((uint32_t*)m_chunk_compressed_data_buffer.GetHostPtr()) + global_compress_address,
                index, m_chunk_bucket.at(index));
        });

        world.SetChunkDeletedCallback([&](VoxelGridSparseT<uint32_t>::ChunkIndexType index) {
            m_sorted_chunk_indices.erase(
                std::find(m_sorted_chunk_indices.begin(), m_sorted_chunk_indices.end(), index));
        });

    }
    else {
        // todo: update only region (?)
        if (m_prev_world_version != world.GetVersion()) {
            m_prev_world_version = world.GetVersion();

            world.WriteGridDataTo((VoxelGridSparseT<uint32_t>::ChunkIndexType*) m_chunk_grid_data_buffer.GetHostPtr());
        }
    }*/


}

void VoxelGridGpuDataManager::CommitChanges(glm::dvec3 observer_position) {
    // TODO: ensure that we have only one VoxelGrid and it is not changing - world grid

    m_lod_manager.CommitChanges();

    RegisterNewEntities();

    assert(m_changes.size() <= 1);

    for (auto&[ent, changes]: m_changes) {
        if (!m_registry.valid(ent)) {
            continue;
        }

        ProcessAllChangesForEntity(ent, changes);
    }

    m_changes.clear();

    //return;

    // Sort chunks
    if (m_sorted_chunk_indices.empty())
        return;

    auto &transform = m_registry.get<TransformComponent>(m_registry.view<VoxelGrid>()[0]);
    auto &grid = m_registry.get<VoxelGrid>(m_registry.view<VoxelGrid>()[0]);
    auto &grid_lod = m_registry.get<VoxelGridLod>(m_registry.view<VoxelGridLod>()[0]);

    // TODO: generalize
    observer_position = transform.ApplyInv(observer_position) * 16.0 + grid.GetAnchor();

    std::vector<double> distance(m_chunk_bucket.size());
    for (uint32_t i = 0; i < m_chunk_bucket.size(); i++) {
        distance.at(i) = glm::length(
                glm::dvec3(glm::dvec3(grid.GetChunkGridPos(i) * VoxelGrid::CHUNK_SIZE) - observer_position));
    }

    // Something like a shell sort.
    // It sorts an array ~approximately~, we do not need precise sort on each step here.
    for (uint32_t step = 128; step >= 1; step >>= 1) {
        for (uint32_t i = m_sorted_chunk_indices.size() - 1; i >= step; i--) {
            if (distance.at(m_sorted_chunk_indices.at(i)) < distance.at(m_sorted_chunk_indices.at(i - step))) {
                std::swap(m_sorted_chunk_indices.at(i), m_sorted_chunk_indices.at(i - step));
            }
        }
    }

    // Move some chunks between buckets depending on their order in sorted list.
    for (int stage = 0; stage < 2; stage++) {

        if (m_current_i >= m_sorted_chunk_indices.size() || true) {
            m_current_i = 0;
            m_current_bucket = 0;
            m_chunks_in_current_bucket = 0;
        }

        for (uint32_t updates = 0;
             m_current_i < m_sorted_chunk_indices.size() && updates < UPDATES_PER_FRAME; m_current_i++) {
            uint32_t index = m_sorted_chunk_indices[m_current_i];
            uint32_t old_bucket = m_chunk_bucket.at(index);

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

            m_chunk_bucket.at(index) = m_current_bucket;
            m_allocator[old_bucket].Free(m_chunk_address.at(index));
            m_chunk_address.at(index) = m_allocator[m_current_bucket].Allocate();

            if (m_chunk_bucket.at(index) == 0) {
                memcpy((uint32_t *) m_chunk_data_buffer.GetHostPtr() + GetGlobalAddress(index),
                       grid.GetChunkViewAsArray(index).Data(),
                       (GetChunkLodSizeDword(m_chunk_bucket.at(index))) * sizeof(uint32_t));
            } else {
                memcpy((uint32_t *) m_chunk_data_buffer.GetHostPtr() + GetGlobalAddress(index),
                       grid_lod.GetChunkViewAtLod(index, m_chunk_bucket.at(index)).Data(),
                       (GetChunkLodSizeDword(m_chunk_bucket.at(index))) * sizeof(uint32_t));
            }

            ((ChunkInfo *) m_chunk_info_buffer.GetHostPtr())[index] = ChunkInfo{GetGlobalAddress(index),
                                                                                m_chunk_bucket.at(index)};

            m_chunks_in_current_bucket++;
        }
    }
}


uint32_t VoxelGridGpuDataManager::GetGlobalAddress(uint32_t index) const {
    return GetBucketOffsetDword(m_chunk_bucket.at(index)) +
           GetChunkLodSizeDword(m_chunk_bucket.at(index)) * m_chunk_address.at(index);
}

UniformBuffer &VoxelGridGpuDataManager::GetChunkGridDataBuffer() {
    return m_chunk_grid_data_buffer;
}

UniformBuffer &VoxelGridGpuDataManager::GetChunkDataBuffer() {
    return m_chunk_data_buffer;
}

UniformBuffer &lit::engine::VoxelGridGpuDataManager::GetChunkCompressedDataBuffer() {
    return m_chunk_bit_data_buffer;
}

uint64_t VoxelGridGpuDataManager::GetWorldLodOffsetDword(int lod) const {
    uint64_t res = 0;
    uint64_t size = 0; // glm::compMul(VoxelGridSparseT<uint32_t>::GetChunkGridDims());
    for (int i = 0; i < lod; i++) {
        res += size;
        size >>= 3; // /=8
    }
    return res;
}

uint64_t VoxelGridGpuDataManager::GetWorldLodSizeDword(int lod) const {
    //return glm::compMul(VoxelGridSparseT<uint32_t>::GetChunkGridDims() >> lod);
    return 0;
}

uint64_t VoxelGridGpuDataManager::GetChunkLodSizeDword(int lod) const {
    return (1 << ((VoxelGridSparseT<uint32_t>::CHUNK_SIZE_LOG - lod) * 3));
}

uint64_t VoxelGridGpuDataManager::GetChunkLodOffsetDword(int bucket, int lod) const {
    uint64_t res = 0;
    for (int i = bucket; i < lod; i++) {
        res += GetChunkLodSizeDword(i);
    }
    return res;
}

uint64_t VoxelGridGpuDataManager::GetChunkSizeDword(int bucket) const {
    uint64_t res = 0;
    for (int i = bucket; i <= VoxelGridSparseT<uint32_t>::CHUNK_SIZE_LOG; i++) {
        res += GetChunkLodSizeDword(i);
    }
    return res;
}

UniformBuffer &VoxelGridGpuDataManager::GetChunkInfoBuffer() {
    return m_chunk_info_buffer;
}

uint64_t VoxelGridGpuDataManager::GetBucketOffsetDword(uint32_t bucket) const {
    uint64_t res = 0;
    for (uint32_t i = 0; i < bucket; i++) {
        res += m_allocator[i].GetSize() * GetChunkLodSizeDword(i);
    }
    return res;
}
