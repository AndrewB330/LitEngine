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

        std::optional<std::tuple<CameraComponent&, TransformComponent&>> GetCamera(entt::registry &registry) const;

        void UpdateShader();

        void UpdateConstantUniforms();

        std::string m_shader_path = ResourcesManager::GetShaderPath("main.glsl");

        ComputeShader m_shader = ComputeShader::Create(m_shader_path);

        UniformBuffer m_global_world_info = UniformBuffer::Create({.size=sizeof(GlobalWorldInfo)});

        VoxelWorldGpuDataManager m_voxel_world_gpu_data_manager;
    };

}