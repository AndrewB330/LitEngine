#pragma once

#include <lit/engine/systems/system.hpp>
#include <lit/rendering/opengl/frame_buffer.hpp>
#include <entt/entt.hpp>
#include <SDL.h>
#include <queue>

namespace lit::engine {

    class EntityView;

    class Scene {
    public:
        Scene() = default;

        bool OnInput(const UserInput& input);

        void OnRedraw(glm::uvec2 viewport, double dt);

        void OnUpdate(double dt);

        template<typename T, typename...Args>
        T& AddSystem(Args &&...args) {
            m_systems.emplace_back(std::make_unique<T>(m_registry, std::forward<Args>(args)...));
            return dynamic_cast<T&>(*m_systems.back());
        }

        template<typename T>
        std::optional<T&> GetSystem() {
            for (auto& system : m_systems) {
                if (T& v = dynamic_cast<T&>(*system)) {
                    return v;
                }
            }
            return std::nullopt;
        }

        EntityView CreteEntity(const std::string& name);

    protected:
        friend class EntityView;

        entt::registry m_registry;

        std::deque<std::unique_ptr<System>> m_systems;
    };

    class EntityView {
    public:
        EntityView() = default;

        template<typename T>
        bool HasComponent() const {
            return m_scene->m_registry.try_get<T>(m_entity) != nullptr;
        }

        template<typename T>
        T& GetComponent() const {
            return m_scene->m_registry.get<T>(m_entity);
        }

        template<typename T, typename ...Args>
        T& AddComponent(Args &&...args) const {
            return m_scene->m_registry.emplace<T>(m_entity, std::forward<Args>(args)...);
        }

        template<typename T>
        T& RemoveComponent() const {
            return m_scene->m_registry.remove<T>(m_entity);
        }

        operator bool() const {
            return m_entity != entt::null && m_scene != nullptr;
        }

        entt::entity GetEntity() const {
            return m_entity;
        }

    private:
        friend class Scene;

        EntityView(entt::entity entity, Scene* scene);

        entt::entity m_entity{ entt::null };
        Scene* m_scene{ nullptr };
    };

}
