#include <lit/engine/systems/voxels/voxel_renderer.hpp>
#include <lit/engine/components/transform.hpp>
#include <lit/engine/components/camera.hpp>
#include <lit/engine/components/voxel_tree.hpp>
#include <GL/glew.h>
#include <lit/viewer/debug_options.hpp>
#include <glm/gtc/integer.hpp>

using namespace lit::engine;

VoxelRenderer::VoxelRenderer() {
    UpdateConstantUniforms();

    auto & world_info = *m_global_world_info.GetHostPtrAs<GlobalWorldInfo>();
    world_info.world_size = VoxelWorld::GetDims();
    world_info.chunk_size = VoxelWorld::GetChunkDims();
    world_info.world_size_log = glm::log2(VoxelWorld::GetDims());
    world_info.chunk_size_log = glm::log2(VoxelWorld::GetChunkDims());
    world_info.world_max_lod = VoxelWorld::WORLD_SIZE_LOG;
    world_info.chunk_max_lod = VoxelWorld::CHUNK_SIZE_LOG;
    int offset = 0;
    for (int i = 0; i < 10; i++) {
        world_info.grid_lod_offset[i] = offset;
        offset += glm::compMul(VoxelWorld::GetChunkGridDims() >> i);
    }
}

void VoxelRenderer::UpdateFrameBuffer(glm::uvec2 viewport) {
    if (m_frame_buffer.GetViewport() != viewport || m_frame_buffer.IsDefault()) {
        m_frame_buffer = std::move(
                FrameBuffer::Create({.width = viewport.x, .height = viewport.y, .attachments = {Attachment::RGBA8}}));
        uint32_t texture_width = (viewport.x + FIRST_PASS_CELL_SIZE - 1) / FIRST_PASS_CELL_SIZE;
        uint32_t texture_height = (viewport.y + FIRST_PASS_CELL_SIZE - 1) / FIRST_PASS_CELL_SIZE;
        m_first_rt_pass_texture = std::move(
                Texture2D::Create(
                        {.internal_format=TextureInternalFormat::R32F, .width=texture_width, .height=texture_height})
        );
    }
}

std::optional<std::tuple<CameraComponent, TransformComponent>>
VoxelRenderer::GetCamera(entt::registry &registry) const {
    for (auto c: registry.view<CameraComponent>()) {
        return std::tuple{registry.get<CameraComponent>(c), registry.get<TransformComponent>(c)};
    }
    return std::nullopt;
}

void VoxelRenderer::UpdateConstantUniforms() {
    m_second_shader.Bind();

    m_camera_info.Bind(1);
    m_global_world_info.Bind(2);

    m_voxel_world_gpu_data_manager.GetWorldDataBuffer().Bind(16);
    m_voxel_world_gpu_data_manager.GetChunkDataBuffer().Bind(17);
    m_voxel_world_gpu_data_manager.GetChunkInfoBuffer().Bind(18);

    m_first_shader.Bind();

    m_camera_info.Bind(1);
    m_global_world_info.Bind(2);

    m_voxel_world_gpu_data_manager.GetWorldDataBuffer().Bind(16);
    m_voxel_world_gpu_data_manager.GetChunkDataBuffer().Bind(17);
    m_voxel_world_gpu_data_manager.GetChunkInfoBuffer().Bind(18);
}

void VoxelRenderer::UpdateShader() {
    auto &dbg = DebugOptions::Instance();
    if (dbg.recompile_shaders) {
        dbg.recompile_shaders = false;
        auto first_recompiled = ComputeShader::TryCreate(m_shader_first_path);
        auto second_recompiled = ComputeShader::TryCreate(m_shader_second_path);
        if (!first_recompiled || !second_recompiled) {
            return;
        }
        auto r1 = std::move(*first_recompiled);
        auto r2 = std::move(*second_recompiled);
        std::swap(m_first_shader, r1);
        std::swap(m_second_shader, r2);
        UpdateConstantUniforms();
    }
}

void VoxelRenderer::Redraw(entt::registry &registry) {
    auto res = GetCamera(registry);

    if (!res) {
        return;
    }

    auto[camera, transform] = *res;

    UpdateFrameBuffer(camera.viewport);
    UpdateShader();

    m_camera_info.GetHostPtrAs<CameraInfo>()->viewport = camera.viewport;
    m_camera_info.GetHostPtrAs<CameraInfo>()->camera_transform = transform.Matrix();
    m_camera_info.GetHostPtrAs<CameraInfo>()->camera_transform_inv = transform.MatrixInv();

    // First cone-pass
/*    m_first_shader.Bind();
    m_first_rt_pass_texture.BindToImage(0);
    m_first_shader.Dispatch(glm::ivec3((camera.viewport.x + FIRST_PASS_CELL_SIZE - 1) / FIRST_PASS_CELL_SIZE, (camera.viewport.y + FIRST_PASS_CELL_SIZE - 1) / FIRST_PASS_CELL_SIZE, 1));
*/
    // Second precise-pass
    m_second_shader.Bind();
    m_frame_buffer.GetAttachmentTextures()[0].lock()->BindToImage(0);
    m_first_rt_pass_texture.BindToImage(1, true);
    m_sky_box.BindToImage(2);
    m_frame_buffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    m_second_shader.Dispatch(glm::ivec3(camera.viewport.x, camera.viewport.y, 1));

    m_frame_buffer.BlitToDefault();
}

void VoxelRenderer::Update(entt::registry &registry, double dt) {
    auto &world = registry.get<VoxelWorld>(registry.view<VoxelWorld>()[0]);
    auto res = GetCamera(registry);

    if (!res) {
        return;
    }

    auto[_, transform] = *res;

    m_voxel_world_gpu_data_manager.Update(world, transform.translation * 16. + glm::dvec3(VoxelWorld::GetDims()) / 2.);
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
