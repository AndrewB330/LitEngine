#include <lit/engine/systems/voxels/voxel_renderer.hpp>
#include <lit/engine/components/transform.hpp>
#include <lit/engine/components/camera.hpp>
#include <lit/engine/components/voxel_grid/voxel_grid_octree.hpp>
#include <GL/glew.h>
#include <lit/viewer/debug_options.hpp>
#include <glm/gtc/integer.hpp>
#include <lit/common/time_utils.hpp>

using namespace lit::engine;

VoxelRenderer::VoxelRenderer() {
    UpdateConstantUniforms();

    auto &world_info = *m_global_world_info.GetHostPtrAs<GlobalWorldInfo>();
    world_info.world_size = VoxelGridSparseT<uint32_t>::GetDims();
    world_info.chunk_size = VoxelGridSparseT<uint32_t>::GetChunkDims();
    world_info.world_size_log = glm::log2(VoxelGridSparseT<uint32_t>::GetDims());
    world_info.chunk_size_log = glm::log2(VoxelGridSparseT<uint32_t>::GetChunkDims());
    world_info.world_max_lod = VoxelGridSparseT<uint32_t>::WORLD_SIZE_LOG;
    world_info.chunk_max_lod = VoxelGridSparseT<uint32_t>::CHUNK_SIZE_LOG;
    int offset = 0;
    for (int i = 0; i < 10; i++) {
        world_info.grid_lod_offset[i] = offset;
        offset += glm::compMul(VoxelGridSparseT<uint32_t>::GetChunkGridDims() >> i);
    }
}

std::optional<std::tuple<CameraComponent&, TransformComponent&>>
VoxelRenderer::GetCamera(entt::registry &registry) const {
    for (auto c: registry.view<CameraComponent>()) {
        return std::tuple{std::ref(registry.get<CameraComponent>(c)), std::ref(registry.get<TransformComponent>(c))};
    }
    return std::nullopt;
}

void VoxelRenderer::UpdateConstantUniforms() {
    m_shader.Bind();

    m_global_world_info.Bind(2);

    m_voxel_world_gpu_data_manager.GetWorldDataBuffer().Bind(16);
    m_voxel_world_gpu_data_manager.GetChunkDataBuffer().Bind(17);
    m_voxel_world_gpu_data_manager.GetChunkCompressedDataBuffer().Bind(18);
    m_voxel_world_gpu_data_manager.GetChunkInfoBuffer().Bind(19);
}

void VoxelRenderer::UpdateShader() {
    auto &dbg = DebugOptions::Instance();
    if (dbg.recompile_shaders) {
        dbg.recompile_shaders = false;
        auto recompiled = ComputeShader::TryCreate(m_shader_path);
        if (!recompiled) {
            return;
        }
        auto r = std::move(*recompiled);
        std::swap(m_shader, r);
        UpdateConstantUniforms();
    }
}


float timee = 0.0;
void VoxelRenderer::Redraw(entt::registry &registry) {
    common::Timer timer;

    auto res = GetCamera(registry);

    if (!res) {
        return;
    }

    auto & [camera, transform] = *res;

    UpdateShader();

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_shader.Bind();

    m_shader.SetUniform("uni_time", timee);

    m_shader.Dispatch(glm::ivec3(camera.GetViewport().x, camera.GetViewport().y, 1));
    timee += timer.GetTimeAndReset() * 1000;

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void VoxelRenderer::Update(entt::registry &registry, double dt) {
    auto &world = registry.get<VoxelGridSparseT<uint32_t>>(registry.view<VoxelGridSparseT<uint32_t>>()[0]);
    auto res = GetCamera(registry);

    if (!res) {
        return;
    }

    auto & [_, transform] = *res;

    m_voxel_world_gpu_data_manager.Update(world, transform.translation * 16. + glm::dvec3(VoxelGridSparseT<uint32_t>::GetDims()) / 2.);
}

/*void VoxelRenderer::RegisterGrid(entt::entity ent, VoxelGrid &grid) {
    if (m_index_mapping.find(ent) != m_index_mapping.end()) return;

    uint32_t index = m_data_addresses.size();

    if (!m_free_indices.empty()) {
        index = *m_free_indices.begin();
        m_free_indices.erase(m_free_indices.begin());
    } else {
        m_data_addresses.emplace_back();
        m_lod_data_addresses.emplace_back();
    }

    m_index_mapping[ent] = index;

    int mip_levels = grid.GetHighestLod() - 1;

    if (!grid.IsBinary()) {
        auto dims = grid.GetDims();
        auto address = m_allocator3D.Allocate(dims);
        m_data_addresses[index] = address;
        m_global_color_texture_3d.Update(grid.m_data, {glm::ivec3(0), dims}, {address, address + dims});
    }

    for (int i = 0; i <= mip_levels; i++) {
        auto dims = grid.GetDimsLod(i);
        auto address = m_allocator3D.Allocate(dims);
        m_lod_data_addresses[index][i] = address;
        m_global_color_texture_3d.Update(grid.m_lod_data[i], {glm::ivec3(0), dims},
                                         {address, address + dims});
    }
}

void VoxelRenderer::UnregisterGrid(entt::entity ent) {
    auto it = m_index_mapping.find(ent);
    if (it != m_index_mapping.end()) {
        m_free_indices.insert(it->second);
        m_index_mapping.erase(it);
    }
}

void VoxelRenderer::UpdateGrid(entt::entity ent, VoxelGrid &grid, const VoxelGridChanges &changes) {
    if (changes.num_changes == 0 || m_index_mapping.find(ent) == m_index_mapping.end()) {
        return;
    }

    auto index = m_index_mapping[ent];

    for (int lod = 0; lod < changes.affected_lod_regions.size(); lod++) {
        auto region = changes.affected_lod_regions[lod];
        auto address = m_lod_data_addresses[index][lod];

        region = region.clamped(iregion3(glm::ivec3(0), grid.GetDimsLod(lod)));
        auto dims = region.end - region.begin;

        m_global_color_texture_3d.Update(grid.m_lod_data[lod], region, {address, address + dims});
    }


    if (!grid.IsBinary()) {
        auto region = changes.affected_region;
        auto address = m_data_addresses[index];

        region = region.clamped(iregion3(glm::ivec3(0), grid.GetDims()));
        auto dims = region.end - region.begin;

        m_global_color_texture_3d.Update(grid.m_data, region, {address, address + dims});
    }
}*/
