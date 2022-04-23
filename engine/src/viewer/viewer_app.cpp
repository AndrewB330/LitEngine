#include <lit/viewer/viewer_app.hpp>
#include <lit/application/platform.hpp>
#include <lit/viewer/viewer_window.hpp>
#include <lit/viewer/debug_window.hpp>
#include <lit/engine/systems/observer_input_controller.hpp>
#include <GL/glew.h>
#include <random>
#include <omp.h>
#include <lit/engine/systems/voxels/voxel_world_generator.hpp>
#include <lit/engine/generators/worldgen.hpp>

using namespace lit::application;
using namespace lit::common;
using namespace lit::engine;
using namespace lit::viewer;


void GLAPIENTRY
MessageCallback(GLenum, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar* message, const void*) {
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

void InitScene(Scene& scene, const spdlog::logger_ptr& logger) {
    auto ent = scene.CreteEntity("world");

    auto& world = ent.AddComponent<VoxelGridSparseT<uint32_t>>(glm::ivec3{ 128, 128, 128 }, glm::dvec3{ 64.0, 0.0, 64.0 });
    ent.AddComponent<VoxelGridSparseLodDataT<uint32_t>>();

    WorldGen worldGen;

    worldGen.ResetTestWorld(world);
    /*
    for (int i = 0; i < world.GetDimensions().x; i++) {
            for(int k = 0; k < world.GetDimensions().z; k++) {
                for(int j = 0; j < world.GetDimensions().y; j++) {
                if ((i - 64) * (i - 64) + (j - 64) * (j - 64) + (k - 64) * (k - 64)  < 64*64)
                    world.SetVoxel({i, j, k}, (i << 16) | (j << 8) | k);
                if (j < sin(i / 128.0) * 64 + cos(k / 128.0) * 64 + 128) {
                    world.SetVoxel({i, j, k}, 0xFFFFFF);
                } else {
                    break;
                }
            }
        }
    }*/

    Timer timer;
    //world = VoxelWorldGenerator::Generate();

}

void ViewerApp::StartApp(const spdlog::logger_ptr& logger) {

    //TestTree();

    const std::string compiler = LIT_COMPILER;
    const std::string architecture = LIT_ARCHITECTURE;
    const std::string config = LIT_CONFIG;

    Application app;
    app.Init();

    Scene scene;
    InitScene(scene, logger);

    WindowInfo game_window;
    game_window.title = "VoxelViewer (" + compiler + " " + architecture + " " + config + ")";
    game_window.maximized = false;
    game_window.width = 1920;
    game_window.height = 1080;

    auto window = std::make_shared<ViewerWindow>(scene);
    auto debug = std::make_shared<DebugUI>();
    app.CreateWindow(game_window, { window, debug}, {debug, window});
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
