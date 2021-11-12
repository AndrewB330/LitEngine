#pragma once

#include <lit/application/application.hpp>

namespace lit::viewer {

    class ViewerApp {
    public:
        ViewerApp() = default;

        void StartApp(const spdlog::logger_ptr& logger);

    };

}
