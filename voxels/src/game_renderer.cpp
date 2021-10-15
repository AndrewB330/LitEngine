#include "game_renderer.hpp"
#include "gpu_data_managers/voxel_data_manager.hpp"

//#include <lit/common/logging.hpp>
#include <utility>

using namespace lit::voxels;

//using lit::common::Logger;

void GLAPIENTRY
MessageCallback(GLenum, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar *message, const void *) {
    if (type == 0x8251) {
        return;
    }
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
    //lit::common::Logger::LogError(message);
}

const std::string assets_path = "../assets/";

GameComponent::GameComponent(std::shared_ptr<lit::voxels::Camera> camera):m_camera(std::move(camera)) {

}

bool GameComponent::Init() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, nullptr);

    m_ctx = std::make_shared<gl::Context>(m_context, spdlog::default_logger());
    VoxelDataManager::Instance().SetContext(m_ctx);

    InitPipeline();

    //Logger::LogInfo("Pipeline initialized");

    InitWorld();



    m_physics = std::make_unique<PhysicsEngine>();
    m_physics->SetGravity(glm::vec3(0, -1, 0));

    std::unique_ptr<PhysObject> floor_object = CreateBoxObject(glm::vec3(25, 4, 25), PhysMaterial::CONCRETE, glm::vec3(0, -2+0.3, 0));
    floor_object->SetFixed(true);

    std::unique_ptr<PhysObject> box_object = CreateBoxObject(glm::vec3(1, 1, 1), PhysMaterial::CONCRETE, glm::vec3(7, 4, -7), glm::vec3(), glm::normalize(glm::quat(4, 1, 2, 3)));

    m_box_collider = box_object->GetCollider();

    std::unique_ptr<PhysObject> box_object2 = CreateBoxObject(glm::vec3(1, 1, 1), PhysMaterial::CONCRETE, glm::vec3(7, 6, -7), glm::vec3(), glm::normalize(glm::quat(4, 3, 2, 1)));

    m_box_collider2 = box_object2->GetCollider();

    m_physics->AddObject(std::move(floor_object));
    m_physics->AddObject(std::move(box_object));
    m_physics->AddObject(std::move(box_object2));

    //Logger::LogInfo("World initialized");

    return true;
}

void GameComponent::Redraw() {
    int width, height;
    SDL_GetWindowSize(m_sdl_window, &width, &height);

    // m_world->GetObjects()[1]->TransformDeferred(vec3(), glm::angleAxis(0.01f, vec3(1, 1, 1)), 1);

    auto rotation = m_box_object->GetTransform().rotation;
    auto rotation_inv = glm::inverse(rotation);
    m_box_object->TransformDeferred(glm::vec3(m_box_collider->translation) - m_box_object->GetTransform().translation, glm::quat(m_box_collider->rotation) * rotation_inv, 1.0);

    auto rotation2 = m_box_object2->GetTransform().rotation;
    auto rotation_inv2 = glm::inverse(rotation2);
    m_box_object2->TransformDeferred(glm::vec3(m_box_collider2->translation) - m_box_object2->GetTransform().translation, glm::quat(m_box_collider2->rotation) * rotation_inv2, 1.0);

    m_world->Update();

    m_physics->Update(0.04);

    m_pipeline->Run(*m_camera, *m_world);
}

glm::vec2 GameComponent::GetViewport() const {
    int width, height;
    SDL_GetWindowSize(m_sdl_window, &width, &height);
    return glm::vec2(width, height);
}

void GameComponent::InitPipeline() {
    m_pipeline = std::make_unique<VoxelsPipeline>(m_ctx);
}

void GameComponent::InitWorld() {
    m_world = std::make_unique<VoxelWorld>(glm::ivec3(512, 256, 512), 0.05f);

    auto model = std::make_shared<VoxelObject>(
            transform3(glm::vec3(0, 0, 0), glm::quat(1, 0, 0, 0), 0.1f),
            glm::ivec3(128, 0, 128),
            glm::ivec3(256, 64, 256)
    );

    m_box_object = std::make_shared<VoxelObject>(
            transform3(glm::vec3(7, 4, -7), glm::quat(4, 1, 2, 3), 0.1f),
            glm::ivec3(8, 8, 8),
            glm::ivec3(16, 16, 16)
    );

    m_box_object2 = std::make_shared<VoxelObject>(
            transform3(glm::vec3(7, 6, -7), glm::quat(4, 3, 2, 1), 0.1f),
            glm::ivec3(8, 8, 8),
            glm::ivec3(16, 16, 16)
    );

    ReadFromFile(*model, assets_path + "test_land_ready_raw.txt");

    ReadFromFile(*m_box_object, assets_path + "box_empty_raw.txt");
    ReadFromFile(*m_box_object2, assets_path + "box_empty_raw.txt");

    m_world->AddObject(model);
    m_world->AddObject(m_box_object);
    m_world->AddObject(m_box_object2);
}
