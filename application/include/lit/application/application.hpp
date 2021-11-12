#pragma once

#include <string>
#ifdef _MSC_VER
#include <SDL_config_winrt.h>
#include <SDL.h>
#elif defined(__MINGW32__)
#include <SDL.h>
#endif
#include <memory>
#include <vector>
#include "window.hpp"

namespace lit::application {

    class Application {
    public:
        explicit Application(spdlog::logger_ptr logger = spdlog::default_logger());

        ~Application();

        bool Init();

        bool CreateWindow(
                const WindowInfo &window_info,
                std::vector<std::shared_ptr<WindowRenderer>> renderers = {},
                std::vector<std::shared_ptr<WindowListener>> listeners = {}
        );

        void PollEvents();

        void Redraw();

        bool AnyWindowAlive() const;

    private:
        bool initialized = false;

        std::vector<Window> windows;

        spdlog::logger_ptr m_logger;
    };

}
