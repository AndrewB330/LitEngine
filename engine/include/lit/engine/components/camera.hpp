#pragma once

#include <lit/engine/components/transform.hpp>
#include <glm/vec2.hpp>

namespace lit::engine {

    struct CameraComponent {
        glm::uvec2 viewport = {1920, 1080};

        double z_near = 0.01f;
        double z_far = 100.0f;
        double field_of_view = 90.0f;

        glm::dmat4 GetProjectionMatrix() const;
    };

}
