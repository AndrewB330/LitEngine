#pragma once

#include <lit/application/window_renderer.hpp>
#include <lit/common/time_utils.hpp>
#include <GL/glew.h>
#include <imgui.h>
#include <memory>
#include "debug_options.hpp"

namespace lit::voxels {

    using lit::common::fps_timer;
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

        fps_timer fps_meter{};
    };

} // namespace LiteEngine::VoxelWorld
