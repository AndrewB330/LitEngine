#include <lit/engine/components/transform.hpp>
#include <lit/engine/components/tag.hpp>
#include <lit/engine/components/camera.hpp>
#include <lit/engine/scene.hpp>


using namespace lit::engine;


bool Scene::OnInput(const UserInput &input) {
    for (auto &system: m_systems) {
        auto input_system = dynamic_cast<UserInputSystem *>(system.get());
        if (input_system && input_system->ProcessInput(input)) {
            return true;
        }
    }

    return false;
}

void Scene::OnRedraw(glm::uvec2 viewport, double dt) {

    // todo: update all cameras that bind to monitor viewport
    for(auto cam : m_registry.view<CameraComponent>()) {
        m_registry.get<CameraComponent>(cam).SetViewport(viewport);
    }

    for (auto &system: m_systems) {
        auto rendering_system = dynamic_cast<RenderingSystem *>(system.get());
        // todo: add dt time offset
        if (rendering_system) {
            rendering_system->Redraw(dt);
        }
    }
}

void Scene::OnUpdate(double dt) {
    for (auto &system: m_systems) {
        auto basic_system = dynamic_cast<BasicSystem *>(system.get());
        if (basic_system) {
            basic_system->Update(dt);
        }
    }
}

EntityView Scene::CreteEntity(const std::string &name) {
    auto ent = m_registry.create();
    m_registry.emplace<TransformComponent>(ent);
    m_registry.emplace<TagComponent>(ent);
    m_registry.get<TagComponent>(ent).tag =
            (name.empty() ? "Entity " + std::to_string(m_registry.size()) : name);
    return {ent, this};
}

lit::engine::EntityView::EntityView(entt::entity entity, Scene* scene):m_entity(entity), m_scene(scene)
{
}
