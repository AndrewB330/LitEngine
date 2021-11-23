#pragma once

#include <lit/engine/systems/renderers/sky_box_renderer.hpp>
#include <lit/engine/components/camera.hpp>
#include <lit/engine/components/sky_box.hpp>
#include <GL/glew.h>

using namespace lit::engine;
using namespace lit::rendering::opengl;

lit::engine::SkyBoxRenderer::SkyBoxRenderer(entt::registry& registry) : System(registry) {}

void SkyBoxRenderer::Redraw(double dt) {
    for (auto entity : m_registry.view<CameraComponent, TransformComponent, SkyBoxComponent>()) {
        auto& camera = m_registry.get<CameraComponent>(entity);
        auto& sky_box = m_registry.get<SkyBoxComponent>(entity);
        auto& transform = m_registry.get<TransformComponent>(entity);

        sky_box.GetTextureCube().BindToImage(2);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

        m_shader.Bind();
        m_shader.Dispatch(glm::ivec3(camera.GetViewport(), 1));
    }
}
