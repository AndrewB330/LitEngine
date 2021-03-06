#pragma once

#include <lit/application/window_listener.hpp>
#include <glm/vec3.hpp>
#include <entt/entt.hpp>
#include "system.hpp"
#include "observer_input_controller.hpp"

namespace lit::engine {

    class ObserverInputController : public UserInputSystem, public BasicSystem {
    public:
        explicit ObserverInputController(entt::registry & registry, entt::entity player);

        ~ObserverInputController() override = default;

        bool ProcessInput(const UserInput &input) override;

        void Update(double dt) override;

    private:
        entt::entity m_player;

        std::unordered_map<int, bool> m_pressed;

        double m_pitch = 0.0f;
        double m_yaw = 0.0f;
        double m_roll = 0.0f;

        glm::dvec3 m_velocity = glm::dvec3();
        double m_acceleration_factor = 30.0;
        double m_friction_factor = 0.01;

        bool m_active = false;
    };

}

