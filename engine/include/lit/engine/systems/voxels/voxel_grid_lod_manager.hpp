#pragma once

#include <lit/engine/systems/system.hpp>
#include <lit/engine/components/voxel_grid/voxel_grid_sparse.hpp>
#include <lit/engine/components/voxel_grid/voxel_grid_sparse_lod_data.hpp>
#include <entt/entt.hpp>

namespace lit::engine {

    template<typename VoxelType>
    class VoxelGridLodManager : public System {
    public:
        VoxelGridLodManager(entt::registry& registry) : System(registry) {}

        ~VoxelGridLodManager() override {
            for (auto ent : m_registry.view<VoxelGrid>()) {
                VoxelGrid& grid = m_registry.get<VoxelGrid>(ent);
                grid.RemoveOnChunkAnyChangeCallback(m_callback_handle[ent]);
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

            if (changes.empty()) {
                return;
            }

            bool chunk_grid_updated = std::any_of(changes.begin(), changes.end(), [](ChunkAnyChangeArgs args) {
                return std::holds_alternative<ChunkCreatedArgs>(args) || std::holds_alternative<ChunkDeletedArgs>(args);
            });

            if (grid_lod.m_grid_lod_data.empty() || chunk_grid_updated) {
                // update grid lods!!!
            }

            std::unordered_set<VoxelGrid::ChunkIndexType> chunks_to_update;

            for (auto& change : m_changes) {
                if (std::holds_alternative<ChunkCreatedArgs>(args)) {
                    m_chunks_to_update.insert(std::get<ChunkCreatedArgs>(args).index);
                }
                else if (std::holds_alternative<ChunkChangedArgs>(args)) {
                    m_chunks_to_update.insert(std::get<ChunkChangedArgs>(args).index);
                }
                else if (std::holds_alternative<ChunkDeletedArgs>(args)) {
                    auto it = m_chunks_to_update.find(std::get<ChunkDeletedArgs>(args).index);
                    if (it) {
                        m_chunks_to_update.erase(it);
                    }
                }
            }

            for (auto& index : chunks_to_update) {

            }
        }

        std::unordered_map<entt::entity, std::vector<ChunkAnyChangeArgs>> m_changes;

        std::unordered_map<entt::entity, size_t> m_callback_handle;
    };

}
