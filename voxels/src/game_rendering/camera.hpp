#pragma once

#include <unordered_map>

#include <lit/common/time_utils.hpp>
#include <lit/common/glm_ext/transform.hpp>

namespace lit::voxels {

    using lit::common::glm_ext::transform3;

    class Camera {
    public:
        struct Viewport {
            uint32_t width;
            uint32_t height;
        };

        explicit Camera(Viewport viewport, glm::vec3 position = glm::vec3());

        void SetPosition(glm::vec3 position);

        void SetRotation(glm::quat rotation);

        void SetViewport(Viewport viewport);

        glm::vec3 GetPosition() const;

        Viewport GetViewport() const;

        glm::vec2 GetViewportSize() const;

        glm::mat4 GetFrustumMat() const;

        glm::mat4 GetViewMat() const;

    private:
        transform3 m_transform;
        Viewport m_viewport;

        const float m_z_near = 0.01f;
        const float m_z_far = 100.0f;
        const float m_fov = 90.0f;
    };

} // namespace LiteEngine::VoxelWorld
