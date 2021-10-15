#pragma once

#include <unordered_map>
#include <memory>

#include <lit/common/time_utils.hpp>
#include <lit/common/glm_ext/transform.hpp>
#include <lit/application/window_listener.hpp>
#include "../game_rendering/camera.hpp"

namespace lit::voxels {

    using lit::application::WindowListener;
    using lit::common::timer;
    using lit::common::glm_ext::transform3;

    class PlayerController : public WindowListener {
    public:
        explicit PlayerController(std::shared_ptr<Camera> camera);

        bool ProcessEvent(const SDL_Event &event) override;

        void StartFrameEvent() override;

        void LookAt(glm::vec3 target);

        void SetPosition(glm::vec3 position);

    private:
        void UpdatePosition();

        std::shared_ptr<Camera> m_camera;

        transform3 transform;

        bool active = false;

        glm::vec3 velocity = glm::vec3();
        float acceleration_factor = 30.0f;
        float friction_factor = 0.01f;

        std::unordered_map<int, bool> pressed;

        float pitch = M_PI * -0.4f;
        float yaw = 0.0f;
        float roll = 0.0f;

        timer m_timer;
    };

} // namespace LiteEngine::VoxelWorld
