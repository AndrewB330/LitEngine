#pragma once

#include <entt/entt.hpp>
#include <lit/engine/algorithms/allocator.hpp>
#include <lit/engine/resources_manager.hpp>
#include <lit/engine/components/voxel_world.hpp>
#include <lit/rendering/opengl/api.hpp>
#include <lit/rendering/opengl/shader.hpp>
#include <lit/rendering/opengl/frame_buffer.hpp>
#include <lit/rendering/opengl/texture.hpp>
#include <lit/rendering/opengl/uniform_buffer.hpp>
#include <lit/engine/components/camera.hpp>
#include "lit/engine/systems/system.hpp"
#include "voxel_world_gpu_data_manager.hpp"

namespace lit::engine {

    using namespace lit::rendering::opengl;

    class VoxelRenderer : public RenderingSystem, public BasicSystem {
    public:
        VoxelRenderer();

        ~VoxelRenderer() override = default;

        void Redraw(entt::registry &registry) override;

        void Update(entt::registry &registry, double dt) override;

    private:

        struct GlobalWorldInfo {
            glm::ivec3 world_size;
            int __pad0;
            glm::ivec3 chunk_size;
            int __pad1;
            glm::ivec3 world_size_log;
            int __pad2;
            glm::ivec3 chunk_size_log;
            int world_max_lod;
            int chunk_max_lod;
            int grid_lod_offset[10];
        };

        struct CameraInfo {
            glm::ivec2 viewport;
            int __pad[2];
            glm::mat4 camera_transform;
            glm::mat4 camera_transform_inv;
        };

        void UpdateFrameBuffer(glm::uvec2 viewport);

        std::optional<std::tuple<CameraComponent, TransformComponent>> GetCamera(entt::registry &registry) const;

        void UpdateShader();

        void UpdateConstantUniforms();

        const int FIRST_PASS_CELL_SIZE = 8;

        std::string m_shader_first_path = ResourcesManager::GetShaderPath("first_pass.glsl");
        std::string m_shader_second_path = ResourcesManager::GetShaderPath("second_pass.glsl");

        ComputeShader m_first_shader = ComputeShader::Create(m_shader_first_path);
        ComputeShader m_second_shader = ComputeShader::Create(m_shader_second_path);

        FrameBuffer m_frame_buffer = FrameBuffer::Default();
        UniformBuffer m_camera_info = UniformBuffer::Create({.size=sizeof(CameraInfo), .coherent=true});
        UniformBuffer m_global_world_info = UniformBuffer::Create({.size=sizeof(GlobalWorldInfo), .coherent=true});

        Texture2D m_first_rt_pass_texture = Texture2D::Create(Texture2DInfo{});

        VoxelWorldGpuDataManager m_voxel_world_gpu_data_manager;
    };

}