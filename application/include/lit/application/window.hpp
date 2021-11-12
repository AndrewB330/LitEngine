#pragma once

#include <string>
#include <memory>
#include <vector>

#ifdef _MSC_VER
#include <SDL_config_winrt.h>
#include <SDL.h>
#elif defined(__MINGW32__)
#include <SDL.h>
#endif

#include <spdlog/spdlog.h>

#include "window_listener.hpp"
#include "window_renderer.hpp"

namespace spdlog {
    using logger_ptr = std::shared_ptr<spdlog::logger>;
}

namespace lit::application {

    struct WindowInfo {
        std::string title = "Default";
        uint32_t width = 800;
        uint32_t height = 600;
        bool maximized = true;
        bool resizable = true;
    };

    class Window {
    public:
        Window(const Window &window) = delete;

        Window(Window &&window) noexcept;

        Window(spdlog::logger_ptr logger, WindowInfo info);

        ~Window();

        bool Init();

        void Redraw();

        bool ProcessEvent(const SDL_Event &event);

        bool IsClosed() const;

        void AddRenderer(std::shared_ptr<WindowRenderer> renderer);

        void AddListener(std::shared_ptr<WindowListener> listener);

    private:
        WindowInfo m_info = WindowInfo();

        SDL_Window *sdl_window = nullptr;
        SDL_GLContext gl_context = nullptr;

        bool initialized = false;
        bool closed = false;

        std::vector<std::shared_ptr<WindowRenderer>> renderers;
        std::vector<std::shared_ptr<WindowListener>> listeners;

        spdlog::logger_ptr m_logger;
    };

}
