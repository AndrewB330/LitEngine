#pragma once

#include "window_listener.hpp"

namespace lit::application {

    class WindowRenderer {
    public:
        WindowRenderer() = default;

        virtual ~WindowRenderer() = default;

        virtual bool Init() = 0;

        virtual void Redraw() = 0;

    protected:
        friend class Window;

        // It is guaranteed that m_sdl_window and m_context is not null for Init or Redraw call
        SDL_Window *m_sdl_window = nullptr;
        SDL_GLContext m_context = nullptr;
    };

}
