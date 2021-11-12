#pragma once

#include <lit/application/window_renderer.hpp>
#include <lit/common/time_utils.hpp>
#include <imgui.h>

namespace lit::viewer {

    using lit::common::FpsTimer;
    using lit::application::WindowRenderer;
    using lit::application::WindowListener;

    class DebugUI : public WindowRenderer, public WindowListener {
    public:
        DebugUI() = default;

        ~DebugUI() override = default;

        bool Init() override;

        void Redraw() override;

        bool ProcessEvent(const SDL_Event &event) override;

        void StartFrameEvent() override;

    private:
        ImGuiContext *imgui_context{};

        FpsTimer fps_meter{};
    };

}