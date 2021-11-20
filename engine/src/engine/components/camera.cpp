#include <lit/engine/components/camera.hpp>

using namespace lit::engine;
using namespace lit::rendering::opengl;

glm::dmat4 CameraComponent::GetProjectionMatrix() const {
    double aspect_ratio = static_cast<float>(GetViewport().x) / static_cast<float>(GetViewport().y);
    return glm::perspective(m_field_of_view, aspect_ratio, m_z_near, m_z_far);
}

CameraComponent::CameraComponent(glm::uvec2 viewport)
        : m_frame_buffer(OpenglFrameBuffer::Create(
        {
                .width=viewport.x,
                .height=viewport.y,
                .attachments={Attachment::RGBA32F, Attachment::R32F}
        })) {}

void CameraComponent::SetViewport(glm::uvec2 viewport) {
    if (viewport != GetViewport()) {
        m_frame_buffer = OpenglFrameBuffer::Create(
                {
                        .width=viewport.x,
                        .height=viewport.y,
                        .attachments={Attachment::RGBA32F, Attachment::R32F}
                });
    }
}

glm::uvec2 CameraComponent::GetViewport() const {
    return m_frame_buffer.GetViewport();
}

const CameraComponent::OpenglFrameBuffer &CameraComponent::GetFrameBuffer() const {
    return m_frame_buffer;
}
