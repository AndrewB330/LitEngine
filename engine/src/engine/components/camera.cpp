#include <lit/engine/components/camera.hpp>

using namespace lit::engine;

glm::dmat4 CameraComponent::GetProjectionMatrix() const {
    double aspect_ratio = static_cast<float>(viewport.x) / static_cast<float>(viewport.y);
    return glm::perspective(field_of_view, aspect_ratio, z_near, z_far);
}
