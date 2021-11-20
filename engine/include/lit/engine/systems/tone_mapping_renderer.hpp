#pragma once

#include <lit/engine/systems/system.hpp>
#include <lit/rendering/opengl/shader.hpp>
#include <lit/engine/resources_manager.hpp>

namespace lit::engine {

    // todo: post processing renderer?
    class ToneMappingRenderer : public RenderingSystem {
    public:
        void Redraw(entt::registry &registry) override;

        ~ToneMappingRenderer() override = default;

    private:
        using ComputeShader = lit::rendering::opengl::ComputeShader;

        ComputeShader m_shader = ComputeShader::Create(ResourcesManager::GetShaderPath("tone_mapping.glsl"));
    };

}