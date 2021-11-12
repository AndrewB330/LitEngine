#include <lit/viewer/viewer_app.hpp>
#include <lit/application/platform.hpp>
#include <lit/viewer/viewer_window.hpp>
#include <lit/viewer/debug_window.hpp>
#include <lit/engine/components/transform.hpp>
#include <lit/common/time_utils.hpp>
#include <GL/glew.h>
#include <lit/engine/entity_view.hpp>
#include <lit/engine/systems/observer_input_controller>
#include <lit/engine/components/camera.hpp>

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

#include <lit/engine/components/voxel_tree.hpp>
void TestTree() {
    VoxelTree tree{glm::ivec3(128)};

    for (int i = 0; i < 128; i++) {
        for(int j = 0; j < 128; j++) {
            for(int k = 0; k < 128; k++) {
                uint32_t val = (i + j * k + k * i + 3 * j);
                if (val % 41 == 0 || val % 97 == 0) {
                    tree.SetVoxel(i, j, k, val);
                }
            }
        }
    }

    /*for (int i = 0; i < 128; i++) {
        for(int j = 0; j < 128; j++) {
            for(int k = 0; k < 128; k++) {
                uint32_t val = (i + j * k + k * i + 3 * j);
                if (val % 25 == 0) {
                    tree.SetVoxel(i, j, k, 0);
                }
            }
        }
    }*/

    for (int i = 0; i < 128; i++) {
        for(int j = 0; j < 128; j++) {
            for(int k = 0; k < 128; k++) {
                uint32_t val = (i + j * k + k * i + 3 * j);
                uint32_t s_val = tree.GetVoxel(i,j,k);
                if (val % 41 == 0 || val % 97 == 0) {
                    if (s_val != val) {
                        spdlog::default_logger()->error("Test failed on: {} {} {}", i, j, k);
                    }
                } else {
                    if (s_val != 0) {
                        spdlog::default_logger()->error("Test failed on: {} {} {}", i, j, k);
                    }
                }
            }
        }
    }
}

std::optional<VoxelChunk> GenerateChunk(glm::ivec3 chunk_pos) {
    float min = 1e9;
    float max = -1e9;

    auto height = [](int x, int y) {
        float radius = glm::length(glm::vec2((x - VoxelWorld::DIMS.x*0.5f), (y -  VoxelWorld::DIMS.z*0.5f)));
        float h = ((-radius * radius)/(2048 * 100 + radius*radius) + 1) * 512;
        float g1 = (x + y*0.2f);
        float g2 = (y - x * 0.1f);
        return sinf(g1 / 48.0f) * 32.0f * cosf(g2 / 46.0f) + h + 42;
    };

    for (int i = 0; i < VoxelChunk::CHUNK_SIZE; i++) {
        for(int j = 0; j < VoxelChunk::CHUNK_SIZE; j++) {
            glm::ivec3 pos = chunk_pos * glm::ivec3(VoxelChunk::CHUNK_SIZE) + glm::ivec3(i, 0, j);
            float val = height(pos.x, pos.z);
            min = std::min(min, val);
            max = std::max(max, val);
        }
    }

    if (max < chunk_pos.y * VoxelChunk::CHUNK_SIZE) {
        return std::nullopt;
    }
    VoxelChunk chunk;
    for (int i = 0; i < VoxelChunk::CHUNK_SIZE; i++) {
        for(int j = 0; j < VoxelChunk::CHUNK_SIZE; j++) {
            glm::ivec3 pos = chunk_pos * glm::ivec3(VoxelChunk::CHUNK_SIZE) + glm::ivec3(i, 0, j);
            float val = height(pos.x, pos.z);
            for(int k = 0; k < VoxelChunk::CHUNK_SIZE; k++) {
                if (k + pos.y < val) {
                    chunk.SetVoxel(i, k, j, 1);
                } else {
                    break;
                }
            }
        }
    }
    chunk.Update();
    return std::move(chunk);
}

Scene InitScene() {
    Scene scene;

    auto & world = scene.CreteEntity("world").AddComponent<VoxelWorld>();

    int a = 0, b = 0;
    while(auto chunk_pos = world.GetNextUnknownChunk()) {
        auto chunk = GenerateChunk(*chunk_pos);
        if (chunk) {
            world.SetChunk(*chunk_pos, std::move(*chunk));
            b++;
        } else {
            world.SetEmpty(*chunk_pos);
            a++;
        }
    }
    world.Update();

    spdlog::default_logger()->info("World created! Empty chunks: {}; Not empty chunks: {};", a, b);

    auto observer = scene.CreteEntity("observer");
    observer.AddComponent<CameraComponent>();

    scene.AddSystem<ObserverInputController>(observer.GetEntity());

    return scene;
}

void ViewerApp::StartApp(const spdlog::logger_ptr &logger) {

    TestTree();

    const std::string compiler = LIT_COMPILER;
    const std::string architecture = LIT_ARCHITECTURE;
    const std::string config = LIT_CONFIG;

    Application app;
    app.Init();

    Scene scene = InitScene();

    WindowInfo game_window;
    game_window.title = "VoxelViewer (" + compiler + " " + architecture + " " + config + ")";
    game_window.maximized = false;
    game_window.width = 1280;
    game_window.height = 720;

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
