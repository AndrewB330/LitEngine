#include <lit/engine/systems/renderers/camera_pre_renderer.hpp>
#include <lit/engine/components/camera.hpp>
#include <GL/glew.h>

using namespace lit::engine;
using namespace lit::rendering::opengl;

struct CameraInfo {
    glm::mat4 camera_transform;
    glm::mat4 camera_transform_inv;
    glm::ivec2 viewport;
};

lit::engine::CameraPreRenderer::CameraPreRenderer(entt::registry& registry): System(registry) {}

void CameraPreRenderer::Redraw(double dt) {
    for(auto entity : m_registry.view<CameraComponent, TransformComponent>()) {
        if (m_indices.find(entity) == m_indices.end()) {
            m_indices[entity] = m_uniform_buffers.size();
            m_uniform_buffers.push_back(UniformBuffer::Create(UniformBufferInfo{.size=sizeof(CameraInfo)}));
        }
        auto & camera = m_registry.get<CameraComponent>(entity);
        auto & transform = m_registry.get<TransformComponent>(entity);

        m_uniform_buffers[m_indices[entity]].Bind(1);

        auto & camera_info = *m_uniform_buffers[m_indices[entity]].GetHostPtrAs<CameraInfo>();
        camera_info.viewport = camera.GetViewport();
        camera_info.camera_transform = transform.Matrix();
        camera_info.camera_transform_inv = transform.MatrixInv();

        camera.GetFrameBuffer().Bind();
        glClear(GL_COLOR_BUFFER_BIT);

        camera.GetFrameBuffer().GetAttachmentTextures()[0].lock()->BindToImage(0, ImageAccess::Write);
        camera.GetFrameBuffer().GetAttachmentTextures()[1].lock()->BindToImage(1, ImageAccess::ReadWrite);
    }
}
