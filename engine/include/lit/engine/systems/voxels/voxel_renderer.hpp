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

        void UpdateFrameBuffer(glm::uvec2 viewport);

        std::optional<std::tuple<CameraComponent, TransformComponent>> GetCamera(entt::registry &registry) const;

        void UpdateShader();

        void UpdateConstantUniforms();

        std::string m_shader_path = ResourcesManager::GetShaderPath("ray_tracing.glsl");

        ComputeShader m_shader = ComputeShader::Create(m_shader_path);
        FrameBuffer m_frame_buffer = FrameBuffer::Default();

        VoxelWorldGpuDataManager m_voxel_world_gpu_data_manager;
    };

}