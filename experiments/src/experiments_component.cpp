#include "experiments_component.hpp"
#include <GL/glew.h>
#include <lit/common/logging.hpp>

void GLAPIENTRY
MessageCallback2(GLenum, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar *message, const void *) {
    if (type == 0x8251) {
        return;
    }
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
    lit::common::Logger::LogError(message);
}

const std::string assets_path = "../assets/";

ExperimentsComponent::ExperimentsComponent(std::shared_ptr<lit::voxels::Camera> camera) : m_camera(std::move(camera)) {}

bool ExperimentsComponent::Init() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback2, nullptr);

    m_ctx = std::make_shared<lit::gl::Context>(m_context);

    InitPipeline();

    lit::common::Logger::LogInfo("Pipeline initialized");

    InitWorld();

    lit::common::Logger::LogInfo("World initialized");

    return true;
}

lit::common::timer t;

void ExperimentsComponent::Redraw() {
    int width, height;
    SDL_GetWindowSize(sdl_window, &width, &height);

    float dt = t.get_elapsed_seconds();

    for(int i = 0; i < 10; i++) {
        m_world->update(dt / 10);
    }

    t = lit::common::timer();

    m_pipeline->Run(*m_camera, *m_world);
}

glm::vec2 ExperimentsComponent::GetViewport() const {
    int width, height;
    SDL_GetWindowSize(sdl_window, &width, &height);
    return glm::vec2(width, height);
}

void ExperimentsComponent::InitPipeline() {
    m_pipeline = std::make_unique<Pipeline>(m_ctx);
}

std::unique_ptr<World> InitClothDemo() {
    auto world = std::make_unique<World>();
    world->boxes.push_back(Box{vec3(5, 1, 5), vec3(0, -0.5f, 0)});
    world->boxes.push_back(Box{vec3(2, 0.4f, 2), vec3(2, 0.2f, 1)});
    world->boxes.push_back(Box{vec3(2, 0.4f, 2), vec3(-1, 0.2f, -1)});
    world->boxes.push_back(Box{vec3(1, 0.2f, 1), vec3(-2.5, 0.1f, 2.5f)});
    world->boxes.push_back(Box{vec3(1.5, 0.6f, 1), vec3(1.5, 0.2f, 1), angleAxis(0.2f, vec3(0, 1, 0))});
    world->boxes.push_back(Box{vec3(1.5, 0.5f, 2), vec3(-1.5, 0.2f, 1), angleAxis(0.5f, vec3(0, 1, 0))});
    world->boxes.push_back(Box{vec3(1.5, 0.5f, 4), vec3(2, 0.2f, -2), angleAxis(0.6f, vec3(0, 1, 0))});

    int cloth_radius = 30;
    float cloth_gap = Particle::k_radius * 2 * 1.1f;

    world->particles.reserve((cloth_radius * 2 + 1) * (cloth_radius * 2 + 1));


    for (int i = -cloth_radius; i <= cloth_radius; i++) {
        for (int j = -cloth_radius; j <= cloth_radius; j++) {
            vec3 pos = vec3(i * cloth_gap + sin(i + j) * 0.000f, 2.0f, j * cloth_gap + sin(j * 3.3f) * 0.000f);
            world->particles.push_back(Particle{pos});
            auto p1 = &world->particles.back();

            if (i > -cloth_radius) {
                int index = (i + cloth_radius - 1) * (2 * cloth_radius + 1) + (j + cloth_radius);
                auto p2 = &world->particles[index];
                world->joints.push_back(Joint{p1, p2, cloth_gap, 0.02f, 0.01f});
            }
            if (j > -cloth_radius) {
                int index = (i + cloth_radius) * (2 * cloth_radius + 1) + (j + cloth_radius - 1);
                auto p2 = &world->particles[index];
                world->joints.push_back(Joint{p1, p2, cloth_gap, 0.02f, 0.01f});
            }
        }
    }
    return std::move(world);
}

void ExperimentsComponent::InitWorld() {
    m_world = InitClothDemo();
    t = lit::common::timer();
}