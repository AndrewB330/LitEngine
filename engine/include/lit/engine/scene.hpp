#pragma once

#include <lit/engine/systems/voxels/voxel_renderer.hpp>
#include <lit/engine/systems/system.hpp>
#include <entt/entt.hpp>
#include <SDL.h>
#include <queue>

namespace lit::engine {

    class EntityView;

    class Scene {
    public:
        Scene() = default;

        bool OnInput(const UserInput &input);

        void OnRedraw(glm::uvec2 viewport, double dt);

        void OnUpdate(double dt);

        template<typename T, typename...Args>
        void AddSystem(Args &&...args) {
            m_systems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        }

        EntityView CreteEntity(const std::string & name);

    protected:
        friend class EntityView;

        entt::registry m_registry;

        std::deque<std::unique_ptr<System>> m_systems;
    };

}
