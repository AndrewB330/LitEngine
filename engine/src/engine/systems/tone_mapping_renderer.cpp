#pragma once

#include <lit/engine/systems/tone_mapping_renderer.hpp>
#include <lit/engine/components/camera.hpp>

using namespace lit::engine;
using namespace lit::rendering::opengl;

void ToneMappingRenderer::Redraw(entt::registry &registry) {
    for(auto entity : registry.view<CameraComponent>()) {
        auto & camera = registry.get<CameraComponent>(entity);

        camera.GetFrameBuffer().GetAttachmentTextures()[0].lock()->BindToImage(0, ImageAccess::ReadWrite);

        m_shader.Bind();
        m_shader.Dispatch(glm::ivec3(camera.GetViewport(), 1));
    }
}
