#pragma once

#include <lit/engine/systems/system.hpp>
#include <lit/rendering/opengl/shader.hpp>
#include <lit/engine/resources_manager.hpp>

namespace lit::engine {

    class SkyBoxRenderer : public RenderingSystem {
    public:

        SkyBoxRenderer(entt::registry& registry);

        void Redraw(double dt) override;

        ~SkyBoxRenderer() override = default;

    private:
        using ComputeShader = lit::rendering::opengl::ComputeShader;

        std::string m_shader_path = ResourcesManager::GetShaderPath("sky_box.glsl");
        ComputeShader m_shader = ComputeShader::Create(m_shader_path);
    };

}