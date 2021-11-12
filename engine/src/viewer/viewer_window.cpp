#include <lit/viewer/viewer_window.hpp>

using namespace lit::viewer;
using namespace lit::engine;

lit::viewer::ViewerWindow::ViewerWindow(Scene &scene) : m_scene(scene) {}

bool lit::viewer::ViewerWindow::Init() {
    m_scene.AddSystem<VoxelRenderer>();
    return true;
}

void ViewerWindow::Redraw() {
    int width, height;
    SDL_GetWindowSize(m_sdl_window, &width, &height);

    m_scene.OnRedraw(glm::uvec2(width, height), 0.0);
}

bool ViewerWindow::ProcessEvent(const SDL_Event &event) {
    return m_scene.OnInput({event});
}

void ViewerWindow::StartFrameEvent() {

}
