#include <lit/engine/entity_view.hpp>

using namespace lit::engine;

EntityView::EntityView(entt::entity entity, Scene *scene) : m_entity(entity), m_scene(scene) {}