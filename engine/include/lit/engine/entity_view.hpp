#pragma once

#include <entt/entt.hpp>
#include <lit/engine/scene.hpp>

namespace lit::engine {

    class EntityView {
    public:
        EntityView() = default;

        template<typename T>
        bool HasComponent() const {
            return m_scene->m_registry.try_get<T>(m_entity) != nullptr;
        }

        template<typename T>
        T &GetComponent() const {
            return m_scene->m_registry.get<T>(m_entity);
        }

        template<typename T, typename ...Args>
        T &AddComponent(Args &&...args) const {
            return m_scene->m_registry.emplace<T>(m_entity, std::forward<Args>(args)...);
        }

        template<typename T>
        T &RemoveComponent() const {
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

        EntityView(entt::entity entity, Scene *scene);

        entt::entity m_entity{entt::null};
        Scene *m_scene{nullptr};
    };
}