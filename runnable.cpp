#define NO_STDIO_REDIRECT
#define GLM_FORCE_SWIZZLE
#define NOMINMAX

#include <lit/application/application.hpp>
#include <lit/common/images/image.hpp>
#include <lit/application/platform.hpp>

#include <game_renderer.hpp>
#include <debug_ui.hpp>

#include <spdlog/spdlog.h>

// Use Discrete GPU
extern "C" { __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001; }

#include <lit/gl/vertex_buffer.hpp>

void RunVoxels() {
    using namespace lit::application;
    using namespace lit::voxels;
    const std::string compiler = LIT_COMPILER;
    const std::string architecture = LIT_ARCHITECTURE;
    const std::string config = LIT_CONFIG;

    Application app;
    app.Init();

    WindowInfo game_window;
    game_window.title = "Voxels preview (" + compiler + " " + architecture + " " + config + ")";
    game_window.maximized = false;
    game_window.width = 1280;
    game_window.height = 720;

    auto camera = std::make_shared<Camera>(Camera::Viewport{game_window.width, game_window.height});
    auto player_controller = std::make_shared<PlayerController>(camera);

    player_controller->SetPosition(glm::vec3(12, 4, -12));
    player_controller->LookAt(glm::vec3());

    auto game = std::make_shared<GameComponent>(camera);
    //auto debug_ui = std::make_shared<DebugUI>();

    app.CreateWindow(game_window, {game}, {player_controller});
    //app.CreateWindow(game_window, {}, {});
    //app.CreateWindow(game_window, {}, {});

    /*using T = lit::gl::VertexData<glm::vec3, glm::vec3>;
    std::vector<T> a(10);
    lit::gl::VertexArray buffer(a);
    buffer.Draw();*/

    while (app.AnyWindowAlive()) {
        app.PollEvents();
        app.Redraw();
    }
}

int main(int, char **) {
    RunVoxels();
    spdlog::default_logger()->dump_backtrace();
    return 0;
}