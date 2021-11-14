#include <lit/viewer/viewer_app.hpp>
#include <lit/application/platform.hpp>
#include <lit/viewer/viewer_window.hpp>
#include <lit/viewer/debug_window.hpp>
#include <lit/engine/entity_view.hpp>
#include <lit/engine/systems/observer_input_controller>
#include <GL/glew.h>
#include <random>
#include <omp.h>
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

/*
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
*/

std::mt19937 rng(1);
std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
float randoms[1024][1024][2];

uint32_t GetColor(glm::vec3 color) {
    color = glm::clamp(color, 0.0f, 1.0f);
    uint32_t r = (color.x) * 255;
    uint32_t g = (color.y) * 255;
    uint32_t b = (color.z) * 255;
    return (r) | (g << 8) | (b << 16);
}

void ResetRandom() {
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 1024; j++) {
            randoms[i][j][0] = distribution(rng);
            randoms[i][j][1] = distribution(rng);
        }
    }
}

void GenerateWorld(VoxelWorld &world) {
    auto grad = [](glm::ivec2 p) {
        float u = randoms[p.x & 1023][p.y & 1023][0];
        float v = randoms[p.x & 1023][p.y & 1023][1];
        return glm::normalize(glm::vec2(u, v) * 2.0f - glm::vec2(1.0f));
    };

    auto fade = [](float t) {
        return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    };

    auto noise = [&grad, &fade](glm::vec2 p) {
        /* Calculate lattice points. */
        glm::ivec2 p0 = floor(p);
        glm::ivec2 p1 = p0 + glm::ivec2(1, 0);
        glm::ivec2 p2 = p0 + glm::ivec2(0, 1);
        glm::ivec2 p3 = p0 + glm::ivec2(1, 1);

        /* Look up gradients at lattice points. */
        glm::vec2 g0 = grad(p0);
        glm::vec2 g1 = grad(p1);
        glm::vec2 g2 = grad(p2);
        glm::vec2 g3 = grad(p3);

        float t0 = p.x - p0.x;
        float fade_t0 = fade(t0); /* Used for interpolation in horizontal direction */

        float t1 = p.y - p0.y;
        float fade_t1 = fade(t1); /* Used for interpolation in vertical direction. */

        /* Calculate dot products and interpolate.*/
        float p0p1 =
                (1.0 - fade_t0) * dot(g0, (p - glm::vec2(p0))) +
                fade_t0 * dot(g1, (p - glm::vec2(p1))); /* between upper two lattice points */
        float p2p3 =
                (1.0 - fade_t0) * dot(g2, (p - glm::vec2(p2))) +
                fade_t0 * dot(g3, (p - glm::vec2(p3))); /* between lower two lattice points */

        /* Calculate final result */
        return (1.0 - fade_t1) * p0p1 + fade_t1 * p2p3;
    };

    std::vector<std::vector<int>> height_map(VoxelWorld::GetDims().x, std::vector<int>(VoxelWorld::GetDims().z));
    std::vector<std::vector<uint32_t>> ground_color(VoxelWorld::GetDims().x, std::vector<uint32_t>(VoxelWorld::GetDims().z));
    std::vector<std::vector<uint32_t>> grass_color(VoxelWorld::GetDims().x, std::vector<uint32_t>(VoxelWorld::GetDims().z));

    ResetRandom();
    #pragma omp parallel for
    for (int i = 0; i < height_map.size(); i++) {
        for (int j = 0; j < height_map[i].size(); j++) {
            glm::vec2 pos{i, j};
            float radius = glm::length(
                    glm::vec2((pos.x - VoxelWorld::GetDims().x * 0.5f), (pos.y - VoxelWorld::GetDims().z * 0.5f)));
            float h = ((-radius * radius) / (2048 * 512 + radius * radius) + 1) * 800;
            height_map[i][j] = (noise(pos / 2048.0f) / 1 +
                                noise(pos / 1024.0f) / 2 +
                                noise(pos / 512.0f) / 4 +
                                noise(pos / 256.0f) / 8 +
                                noise(pos / 128.0f) / 16 +
                                noise(pos / 64.0f) / 32 +
                                noise(pos / 32.0f) / 64 +
                                noise(pos / 16.0f) / 128) * 512 + h - 128;
        }
    }

    ResetRandom();
#pragma omp parallel for
    for (int i = 0; i < ground_color.size(); i++) {
        for (int j = 0; j < ground_color[i].size(); j++) {
            glm::vec2 pos{i, j};
            float c = (float)((noise(pos / 1024.0f) / 1 +
                               noise(pos / 512.0f) / 2 +
                               noise(pos / 256.0f) / 4 +
                               noise(pos / 128.0f) / 8 +
                               noise(pos / 64.0f) / 16 +
                               noise(pos / 32.0f) / 32 +
                               noise(pos / 16.0f) / 64) * 0.5f + 0.5f);
            glm::vec3 color = (glm::vec3(143, 140, 134)/256.0f) * c + (glm::vec3(115, 87, 43)/256.0f) * (1 - c);
            ground_color[i][j] = GetColor(color);
        }
    }

    ResetRandom();
#pragma omp parallel for
    for (int i = 0; i < ground_color.size(); i++) {
        for (int j = 0; j < ground_color[i].size(); j++) {
            glm::vec2 pos{i, j};
            float c = (float)((noise(pos / 1024.0f) / 1 +
                               noise(pos / 512.0f) / 2 +
                               noise(pos / 256.0f) / 4 +
                               noise(pos / 128.0f) / 8 +
                               noise(pos / 64.0f) / 16 +
                               noise(pos / 32.0f) / 32 +
                               noise(pos / 16.0f) / 64) * 0.5f + 0.5f);
            glm::vec3 color = (glm::vec3(82, 207, 29)/256.0f) * c + (glm::vec3(209, 209, 65)/256.0f) * (1 - c);
            grass_color[i][j] = GetColor(color);
        }
    }

    world.SetGenerator([&](glm::ivec3 grid_position, VoxelWorld::ChunkRaw &chunk) {
        for (int i = 0; i < VoxelWorld::CHUNK_SIZE; i++) {
            for (int j = 0; j < VoxelWorld::CHUNK_SIZE; j++) {
                glm::ivec3 pos = grid_position * glm::ivec3(VoxelWorld::CHUNK_SIZE) + glm::ivec3(i, 0, j);
                int val = height_map[pos.x][pos.z];
                int min = val;
                int max = val;
                for(int dx = -4; dx <= 4; dx++) {
                    for(int dy = -4; dy <= 4; dy++) {
                        if (pos.x + dx < 0 || pos.x + dx >= height_map.size()) continue;
                        if (pos.y + dy < 0 || pos.y + dy >= height_map[0].size()) continue;
                        min = std::min(min, height_map[pos.x + dx][pos.z + dy]);
                        max = std::max(max, height_map[pos.x + dx][pos.z + dy]);
                    }
                }
                for (int k = 0; k < VoxelWorld::CHUNK_SIZE; k++) {
                    if (k + pos.y < val) {
                        if (max - min < 9 && max > 600) {
                            chunk[i][k][j] = 0xFFFFFF | (1 << 25);
                        } else if (k + pos.y + 1 >= val && max - min < 7)  {
                            chunk[i][k][j] = grass_color[pos.x][pos.z] | (1 << 25);
                        } else {
                            chunk[i][k][j] = ground_color[pos.x][pos.z] | (1 << 24);
                        };
                    } else {
                        break;
                    }
                }
            }
        }
    });
}

Scene InitScene() {
    Scene scene;
    auto &world = scene.CreteEntity("world").AddComponent<VoxelWorld>();

    int num = 0;

    Timer timer;
    GenerateWorld(world);

    spdlog::default_logger()->info("World created! Chunks: {}; Size: {} MB; Time: {} s",
                                   world.GetChunksNum(), world.GetSize() / 1000000, timer.GetTime());

    auto observer = scene.CreteEntity("observer");
    observer.AddComponent<CameraComponent>();

    scene.AddSystem<ObserverInputController>(observer.GetEntity());

    return scene;
}

void ViewerApp::StartApp(const spdlog::logger_ptr &logger) {

    //TestTree();

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
