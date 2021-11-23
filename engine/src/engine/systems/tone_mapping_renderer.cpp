#pragma once

#include <lit/engine/systems/renderers/tone_mapping_renderer.hpp>
#include <lit/engine/components/camera.hpp>

using namespace lit::engine;
using namespace lit::rendering::opengl;

lit::engine::ToneMappingRenderer::ToneMappingRenderer(entt::registry& registry) : System(registry) {}

void ToneMappingRenderer::Redraw(double dt) {
    for(auto entity : m_registry.view<CameraComponent>()) {
        auto & camera = m_registry.get<CameraComponent>(entity);

        camera.GetFrameBuffer().GetAttachmentTextures()[0].lock()->BindToImage(0, ImageAccess::ReadWrite);

        m_shader.Bind();
        m_shader.Dispatch(glm::ivec3(camera.GetViewport(), 1));
    }
}
