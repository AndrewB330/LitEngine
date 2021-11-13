#pragma pack(push)
#define GLM_FORCE_SWIZZLE
#define NOMINMAX
#define NO_STDIO_REDIRECT
#define HAVE_LIBC 1
#include <SDL_config_winrt.h>
#include <lit/viewer/viewer_app.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <omp.h>
// Use Discrete GPU
extern "C" { __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001; }

int main(int, char **) {
    auto logger = spdlog::default_logger();

    auto file_logger = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log.txt", true);
    logger->sinks().push_back(file_logger);
    logger->set_level(spdlog::level::trace);

    try {
        lit::viewer::ViewerApp().StartApp(logger);
    } catch (const std::exception &e) {
        logger->critical(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}