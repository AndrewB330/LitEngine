#pragma once

#include <lit/engine/systems/system.hpp>
#include <lit/engine/components/voxel_grid/voxel_grid_sparse.hpp>
#include <lit/engine/components/voxel_grid/voxel_grid_sparse_lod_data.hpp>
#include <entt/entt.hpp>

namespace lit::engine {

    template<typename T>
    void ExpandVectorToSize(std::vector<T>& v, size_t size) {
        if (v.size() < size) {
            if (size > v.capacity()) {
                v.reserve(std::max(size, v.capacity() * 3 / 2 + 8));
            }
            v.resize(size);
        }
    }

    template<typename VoxelType>
    class VoxelGridLodManager : public System {
    public:
        VoxelGridLodManager(entt::registry& registry) : System(registry) {}

        ~VoxelGridLodManager() override {
            for (auto ent : m_registry.view<VoxelGrid>()) {
                if (m_callback_handle.find(ent) != m_callback_handle.end()) {
                    VoxelGrid& grid = m_registry.get<VoxelGrid>(ent);
                    grid.RemoveOnChunkAnyChangeCallback(m_callback_handle[ent]);
                }
            }
        }

        void CommitChanges() {
            RegisterNewEntities();

            for (auto& [ent, changes] : m_changes) {
                if (!m_registry.valid(ent)) {
                    continue;
                }

                ProcessAllChangesForEntity(ent, changes);
            }

            m_changes.clear();
        }

    private:

        using VoxelGrid = VoxelGridSparseT<VoxelType>;
        using VoxelGridLod = VoxelGridSparseLodDataT<VoxelType>;
        using ChunkCreatedArgs = VoxelGrid::ChunkCreatedArgs;
        using ChunkChangedArgs = VoxelGrid::ChunkChangedArgs;
        using ChunkDeletedArgs = VoxelGrid::ChunkDeletedArgs;
        using ChunkAnyChangeArgs = std::variant<ChunkCreatedArgs, ChunkChangedArgs, ChunkDeletedArgs>;

        void RegisterNewEntities() {
            for (auto ent : m_registry.view<VoxelGrid, VoxelGridLod>()) {
                auto& grid = m_registry.get<VoxelGrid>(ent);
                auto& grid_lod = m_registry.get<VoxelGridLod>(ent);

                if (!grid_lod.m_grid_lod_data.empty()) {
                    continue;
                }

                grid.InvokeForAllChunks([ent, this](const VoxelGrid::ChunkView& v) {
                    m_changes[ent].push_back(typename VoxelGrid::ChunkCreatedArgs{ v.GetIndex(), v.GetChunkGridPosition() });
                });

                size_t handle = grid.AddOnChunkAnyChangeCallback([ent, this](ChunkAnyChangeArgs args) {
                    m_changes[ent].push_back(args);
                });
            }
        }

        void ProcessAllChangesForEntity(entt::entity& ent, const std::vector<ChunkAnyChangeArgs>& changes) {
            auto& grid_lod = m_registry.get<VoxelGridLod>(ent);
            auto& grid = m_registry.get<VoxelGrid>(ent);

            if (changes.empty()) {
                return;
            }

            bool chunk_grid_updated = std::any_of(changes.begin(), changes.end(), [](ChunkAnyChangeArgs args) {
                return std::holds_alternative<ChunkCreatedArgs>(args) || std::holds_alternative<ChunkDeletedArgs>(args);
            });

            if (grid_lod.m_grid_lod_data.empty() || chunk_grid_updated) {
                if (grid_lod.m_grid_lod_data.empty()) {
                    grid_lod.SetDimensions(grid.GetChunkGridDimensions());
                }
                grid_lod.GetChunkGridViewAtLod(0) = grid.GetChunkGridView();
                glm::ivec3 dims_cur = grid.GetChunkGridDimensions();
                glm::ivec3 dims_next = (dims_cur + 1) >> 1;
                for (int lod = 1; lod <= grid_lod.m_max_grid_lod; lod++) {
                    Array3DView view_cur = grid_lod.GetChunkGridViewAtLod(lod - 1);
                    Array3DView view_next = grid_lod.GetChunkGridViewAtLod(lod);
                    view_next.Fill(0);
                    for (int i = 0; i < dims_cur.x; i++) {
                        for (int j = 0; j < dims_cur.y; j++) {
                            for (int k = 0; k < dims_cur.z; k++) {
                                view_next.At(i >> 1, j >> 1, k >> 1) |= view_cur.At(i, j, k);
                            }
                        }
                    }
                }
            }

            std::unordered_set<VoxelGrid::ChunkIndexType> chunks_to_update;

            typename VoxelGrid::ChunkIndexType max_index = 0;

            for (auto& change : m_changes) {
                if (std::holds_alternative<ChunkCreatedArgs>(change)) {
                    auto index = std::get<ChunkCreatedArgs>(change).index;
                    chunks_to_update.insert(index);
                    max_index = std::max(max_index, index);
                }
                else if (std::holds_alternative<ChunkChangedArgs>(change)) {
                    chunks_to_update.insert(std::get<ChunkChangedArgs>(change).index);
                }
                else if (std::holds_alternative<ChunkDeletedArgs>(change)) {
                    auto it = chunks_to_update.find(std::get<ChunkDeletedArgs>(change).index);
                    if (it != chunks_to_update.size()) {
                        chunks_to_update.erase(it);
                    }
                }
            }

            // expand data storage
            size_t target_size = (max_index + 1) * GetLodTotalSize(VoxelGrid::GetChunkDimensions(), 1, VoxelGrid::CHUNK_SIZE_LOG);
            ExpandVectorToSize(grid_lod.m_chunk_lod_data, target_size);

            size_t target_bit_size = (max_index + 1) * GetLodTotalSize(VoxelGrid::GetChunkDimensions(), 0, VoxelGrid::CHUNK_SIZE_LOG);
            ExpandVectorToSize(grid_lod.m_chunk_binary_lod_data, (target_bit_size + 31) / 32);

            for (auto& index : chunks_to_update) {
                // update regular chunk lods
                for (int lod = 1; lod <= VoxelGrid::CHUNK_SIZE_LOG; lod++) {
                    const Array3DView view_cur = (lod == 1 ? grid_lod.GetChunkViewAtLod(index, lod - 1) : grid.GetChunkViewAsArray(index));
                    Array3DView view_next = grid_lod.GetChunkViewAtLod(index, lod);
                    view_next.Fill(0);
                    for (int i = 0; i < (VoxelGrid::CHUNK_SIZE >> (lod-1)); i++) {
                        for (int j = 0; j < (VoxelGrid::CHUNK_SIZE >> (lod - 1)); j++) {
                            for (int k = 0; k < (VoxelGrid::CHUNK_SIZE >> (lod - 1)); k++) {
                                view_next.At(i >> 1, j >> 1, k >> 1) = std::max(view_cur.At(i, j, k), view_next.At(i >> 1, j >> 1, k >> 1));
                            }
                        }
                    }
                }
                // update binary chunk lods
                const Array3DView viewraw = grid.GetChunkViewAsArray(index);
                Array3DViewBool view0 = grid_lod.GetBinaryChunkAtLod(index, 0);
                for (int i = 0; i < VoxelGrid::CHUNK_SIZE; i++) {
                    for (int j = 0; j < VoxelGrid::CHUNK_SIZE; j++) {
                        for (int k = 0; k < VoxelGrid::CHUNK_SIZE; k++) {
                            view0.Set(i, j, k, viewraw.Get(i, j, k) > 0);
                        }
                    }
                }

                for (int lod = 1; lod <= VoxelGrid::CHUNK_SIZE_LOG; lod++) {
                    Array3DViewBool view_cur = grid_lod.GetBinaryChunkAtLod(index, lod - 1);
                    Array3DViewBool view_next = grid_lod.GetBinaryChunkAtLod(index, lod);
                    view_next.Fill(0);
                    for (int i = 0; i < (VoxelGrid::CHUNK_SIZE >> (lod - 1)); i++) {
                        for (int j = 0; j < (VoxelGrid::CHUNK_SIZE >> (lod - 1)); j++) {
                            for (int k = 0; k < (VoxelGrid::CHUNK_SIZE >> (lod - 1)); k++) {
                                view_next.Set(i >> 1, j >> 1, k >> 1, view_cur.Get(i, j, k) || view_next.Get(i >> 1, j >> 1, k >> 1));
                            }
                        }
                    }
                }
            }
        }

        std::unordered_map<entt::entity, std::vector<ChunkAnyChangeArgs>> m_changes;

        std::unordered_map<entt::entity, size_t> m_callback_handle;
    };

}
