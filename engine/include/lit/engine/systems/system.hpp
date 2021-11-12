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
    };

    class UserInputSystem : virtual public System {
    public:
        virtual bool ProcessInput(entt::registry &registry, const UserInput &input) = 0;

        virtual ~UserInputSystem() = default;
    };

    class RenderingSystem : virtual public System {
    public:
        virtual void Redraw(entt::registry &registry) = 0;

        virtual ~RenderingSystem() = default;
    };

    class BasicSystem : virtual public System {
    public:
        virtual void Update(entt::registry &registry, double delta_time) = 0;

        virtual ~BasicSystem() = default;
    };
}
