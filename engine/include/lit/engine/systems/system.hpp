#pragma once

#include <entt/entt.hpp>
#include <SDL.h>

namespace lit::engine {

    struct UserInput {
        SDL_Event event{};
    };

    class System {
    public:
        virtual ~System() = default;

        System(entt::registry& registry):m_registry(registry) {}

        entt::registry& m_registry;
    };

    class UserInputSystem : virtual public System {
    public:
        virtual bool ProcessInput(const UserInput &input) = 0;

        virtual ~UserInputSystem() = default;
    };

    class RenderingSystem : virtual public System {
    public:
        virtual void Redraw(double delta_time) = 0;

        virtual ~RenderingSystem() = default;
    };

    class BasicSystem : virtual public System {
    public:
        virtual void Update(double delta_time) = 0;

        virtual ~BasicSystem() = default;
    };
}
