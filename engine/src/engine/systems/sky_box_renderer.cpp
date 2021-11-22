#pragma once

#include <lit/engine/systems/renderers/sky_box_renderer.hpp>
#include <lit/engine/components/camera.hpp>
#include <lit/engine/components/sky_box.hpp>
#include <GL/glew.h>

using namespace lit::engine;
using namespace lit::rendering::opengl;

void SkyBoxRenderer::Redraw(entt::registry &registry) {
    for(auto entity : registry.view<CameraComponent, TransformComponent, SkyBoxComponent>()) {
        auto & camera = registry.get<CameraComponent>(entity);
        auto & sky_box = registry.get<SkyBoxComponent>(entity);
        auto & transform = registry.get<TransformComponent>(entity);

        sky_box.GetTextureCube().BindToImage(2);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

        m_shader.Bind();
        m_shader.Dispatch(glm::ivec3(camera.GetViewport(), 1));
    }
}
