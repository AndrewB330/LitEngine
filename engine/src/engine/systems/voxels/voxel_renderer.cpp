#include <lit/engine/systems/voxels/voxel_renderer.hpp>
#include <lit/engine/components/transform.hpp>
#include <lit/engine/components/camera.hpp>
#include <lit/engine/components/voxel_tree.hpp>
#include <GL/glew.h>
#include <lit/viewer/debug_options.hpp>

using namespace lit::engine;

VoxelRenderer::VoxelRenderer() {
    UpdateConstantUniforms();
}

void VoxelRenderer::UpdateFrameBuffer(glm::uvec2 viewport) {
    if (m_frame_buffer.GetViewport() != viewport || m_frame_buffer.IsDefault()) {
        m_frame_buffer = std::move(
                FrameBuffer::Create({.width = viewport.x, .height = viewport.y, .attachments = {Attachment::RGBA8}}));
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
    m_shader.Bind();
    m_shader.SetUniform("uni_world_size", (glm::ivec3) VoxelWorld::GetDims());
    m_shader.SetUniform("uni_chunk_size", (glm::ivec3) glm::ivec3(VoxelWorld::CHUNK_SIZE));
    m_shader.SetUniform("uni_world_max_lod", (int) VoxelWorld::WORLD_SIZE_LOG);
    m_shader.SetUniform("uni_chunk_max_lod", (int) VoxelWorld::CHUNK_SIZE_LOG);

    for (int i = 0; i < 10; i++) {
        int location = m_shader.GetUniformLocation("uni_world_lod_buf_offset[0]");
        if (location == -1) break;
        m_shader.SetUniform(location + i, (int) m_voxel_world_gpu_data_manager.GetWorldLodOffsetDword(i));
    }
    for (int i = 0; i < 10; i++) {
        int location = m_shader.GetUniformLocation("uni_world_linearizer[0]");
        if (location == -1) break;
        glm::ivec3 size = VoxelWorld::GetChunkGridDims() >> i;
        m_shader.SetUniform(location + i, (glm::ivec2) glm::ivec2(size.y * size.z, size.z));
    }
    for (int i = 0; i < 10; i++) {
        int location = m_shader.GetUniformLocation("uni_chunk_lod_buf_offset[0]");
        if (location == -1) break;
        m_shader.SetUniform(location + i, (int) m_voxel_world_gpu_data_manager.GetChunkLodOffsetDword(0, i));
    }
    for (int i = 0; i < 10; i++) {
        int location = m_shader.GetUniformLocation("uni_chunk_linearizer[0]");
        if (location == -1) break;
        glm::ivec3 size = glm::ivec3(VoxelWorld::CHUNK_SIZE) >> i;
        m_shader.SetUniform(location + i, (glm::ivec2) glm::ivec2(size.x, size.x * size.y));
    }

    m_voxel_world_gpu_data_manager.GetWorldDataBuffer().Bind(16);
    m_voxel_world_gpu_data_manager.GetChunkDataBuffer().Bind(17);
    m_voxel_world_gpu_data_manager.GetChunkInfoBuffer().Bind(18);
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

void VoxelRenderer::Redraw(entt::registry &registry) {
    auto res = GetCamera(registry);

    if (!res) {
        return;
    }

    auto[camera, transform] = *res;

    UpdateFrameBuffer(camera.viewport);
    UpdateShader();

    m_shader.Bind();
    m_shader.SetUniform("uni_camera_transform", (glm::mat4) transform.Matrix());
    m_shader.SetUniform("uni_camera_transform_inv", (glm::mat4) transform.MatrixInv());
    m_shader.SetUniform("uni_viewport", (glm::ivec2) camera.viewport);

    m_frame_buffer.GetAttachmentTextures()[0].lock()->BindToImage(0);

    m_frame_buffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT);

    m_shader.Dispatch(glm::ivec3(camera.viewport.x, camera.viewport.y, 1));

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
