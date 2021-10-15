#include "camera.hpp"
#include <SDL2/SDL_stdinc.h>

using namespace lit::voxels;

Camera::Camera(Camera::Viewport viewport, glm::vec3 position)
        : m_viewport(viewport),
          m_transform(position, glm::quat(1, 0, 0, 0), 1.0f) {

}

void Camera::SetPosition(glm::vec3 position) {
    m_transform.translation = position;
}

void Camera::SetRotation(glm::quat rotation) {
    m_transform.rotation = rotation;
}

void Camera::SetViewport(Camera::Viewport viewport) {
    m_viewport = viewport;
}

Camera::Viewport Camera::GetViewport() const {
    return m_viewport;
}

glm::mat4 Camera::GetFrustumMat() const {
    const float h = tanf(m_fov / 360.0f * static_cast<float>(M_PI)) * m_z_near;

    float aspect = static_cast<float>(m_viewport.width) / static_cast<float>(m_viewport.height);
    float w = h * aspect;

    return glm::frustum(-w, w, -h, h, m_z_near, m_z_far);
}

glm::mat4 Camera::GetViewMat() const {
    return m_transform.mat_inv();
}

glm::vec2 Camera::GetViewportSize() const {
    return glm::vec2(m_viewport.width, m_viewport.height);
}

glm::vec3 Camera::GetPosition() const {
    return m_transform.translation;
}
