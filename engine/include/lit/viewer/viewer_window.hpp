#pragma once

#include <lit/application/window_renderer.hpp>
#include <lit/engine/systems/voxels/voxel_renderer.hpp>
#include <lit/engine/scene.hpp>
#include <memory>

namespace lit::viewer {

    class ViewerWindow : public lit::application::WindowRenderer, public lit::application::WindowListener {
    public:
        explicit ViewerWindow(lit::engine::Scene &scene);

        ~ViewerWindow() override = default;

        bool Init() override;

        bool ProcessEvent(const SDL_Event &event) override;

        void StartFrameEvent() override;

        void Redraw() override;

    private:

        lit::engine::Scene& m_scene;
    };

}