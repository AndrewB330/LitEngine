#include <lit/viewer/viewer_window.hpp>
#include <lit/engine/systems/observer_input_controller.hpp>
#include <lit/engine/systems/renderers/sky_box_renderer.hpp>
#include <lit/engine/components/sky_box.hpp>
#include <lit/engine/systems/renderers/camera_pre_renderer.hpp>
#include <lit/engine/systems/renderers/tone_mapping_renderer.hpp>
#include <lit/engine/systems/voxels/voxel_grid_lod_manager.hpp>

using namespace lit::viewer;
using namespace lit::engine;

lit::viewer::ViewerWindow::ViewerWindow(Scene &scene)
        : m_scene(scene) {}

bool lit::viewer::ViewerWindow::Init() {
    m_observer = m_scene.CreteEntity("observer");
    m_observer.AddComponent<CameraComponent>(glm::uvec2(1280, 720));
    //m_observer.AddComponent<SkyBoxComponent>(ResourcesManager::GetAssetPath("sky_boxes/standard"));

    m_scene.AddSystem<CameraPreRenderer>();
    m_scene.AddSystem<SkyBoxRenderer>();
    m_scene.AddSystem<VoxelRenderer>();
    m_scene.AddSystem<ToneMappingRenderer>();
    m_scene.AddSystem<ObserverInputController>(m_observer.GetEntity());

    auto& lod_manager = m_scene.AddSystem<VoxelGridLodManager<uint32_t>>();



    m_data_manager = &m_scene.AddSystem<VoxelGridGpuDataManager>(lod_manager);
    return true;
}

void ViewerWindow::Redraw() {
    int width, height;
    SDL_GetWindowSize(m_sdl_window, &width, &height);

    m_data_manager->CommitChanges(m_observer.GetComponent<TransformComponent>().translation);
    m_scene.OnRedraw(glm::uvec2(width, height), 0.0);

    m_observer.GetComponent<CameraComponent>().GetFrameBuffer().BlitToDefault();
}

bool ViewerWindow::ProcessEvent(const SDL_Event &event) {
    return m_scene.OnInput({event});
}

void ViewerWindow::StartFrameEvent() {

}
