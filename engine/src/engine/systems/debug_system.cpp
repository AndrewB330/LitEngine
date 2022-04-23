#include <lit/engine/systems/debug_system.hpp>
#include <lit/viewer/debug_options.hpp>
#include <lit/engine/generators/worldgen.hpp>
#include <lit/engine/generators/treegen.hpp>
#include <lit/engine/components/voxel_grid/voxel_grid_sparse.hpp>
#include <lit/engine/components/voxel_grid/voxel_grid_sparse_lod_data.hpp>
#include <lit/engine/utilities/voxel_grid_utils.hpp>
#include <lit/common/random.hpp>

using namespace lit::engine;
using namespace lit::common;

DebugSystem::DebugSystem(entt::registry &registry) : System(registry) {}

using VoxelGrid = VoxelGridSparseT<uint32_t>;
using VoxelGridLod = VoxelGridSparseLodDataT<uint32_t>;

RandomGen rng(0);

void DebugSystem::Update(double dt) {

    auto & world = m_registry.get<VoxelGrid>(m_registry.view<VoxelGrid>()[0]);

    if (DebugOptions::Instance().regenerate_tree) {
        WorldGen().ResetTestWorld(world);

        auto tree = TreeGen(rng.get()).GenerateTreeAny();

        Merge(world, *tree, MERGE_KEEP_SECOND, glm::dvec3{-44.,0.,0.});

        tree = TreeGen(rng.get()).GenerateTreeAny();
        Merge(world, *tree, MERGE_KEEP_SECOND, glm::dvec3{44.,0.,0.});

        tree = TreeGen(rng.get()).GenerateTreeAny();
        Merge(world, *tree, MERGE_KEEP_SECOND, glm::dvec3{0.,0.,44.});

        DebugOptions::Instance().regenerate_tree = false;
    }
}
