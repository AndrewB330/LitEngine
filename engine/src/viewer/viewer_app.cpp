#include <lit/viewer/viewer_app.hpp>
#include <lit/application/platform.hpp>
#include <lit/viewer/viewer_window.hpp>
#include <lit/viewer/debug_window.hpp>
#include <lit/engine/systems/observer_input_controller.hpp>
#include <GL/glew.h>
#include <random>
#include <omp.h>
#include <lit/engine/systems/voxels/voxel_world_generator.hpp>

using namespace lit::application;
using namespace lit::common;
using namespace lit::engine;
using namespace lit::viewer;


void GLAPIENTRY
MessageCallback(GLenum, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar *message, const void *) {
    static int prev_error_type = 0;
    if (type == 0x8251 || prev_error_type == type) {
        return;
    }
    prev_error_type = type;
    spdlog::default_logger()->error("GL CALLBACK: {} type = {}, severity = {}, message = {}",
                                    (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

void EnableGlDebug() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, nullptr);
}

void InitScene(Scene & scene) {
    auto &world = scene.CreteEntity("world").AddComponent<VoxelGridSparseT<uint32_t>>(glm::ivec3{512, 512, 512}, glm::dvec3{0.0, 0.0, 0.0});

    Timer timer;
    world = VoxelWorldGenerator::Generate();

}

void ViewerApp::StartApp(const spdlog::logger_ptr &logger) {

    //TestTree();

    const std::string compiler = LIT_COMPILER;
    const std::string architecture = LIT_ARCHITECTURE;
    const std::string config = LIT_CONFIG;

    Application app;
    app.Init();

    Scene scene;
    InitScene(scene);

    WindowInfo game_window;
    game_window.title = "VoxelViewer (" + compiler + " " + architecture + " " + config + ")";
    game_window.maximized = false;
    game_window.width = 1920;
    game_window.height = 1080;

    auto window = std::make_shared<ViewerWindow>(scene);
    auto debug = std::make_shared<DebugUI>();
    app.CreateWindow(game_window, {window, debug}, {debug, window});
    EnableGlDebug();

    Timer timer;

    /**
     * MAIN LOOP
     * 1. Poll Events for all windows ->
     * 2. Update main scene ->
     * 3. Redraw all windows.
     */
    while (app.AnyWindowAlive()) {
        app.PollEvents();
        scene.OnUpdate(timer.GetTimeAndReset());
        app.Redraw();
    }
}
