#include "player_controller.hpp"

#include <utility>

using namespace lit::voxels;

constexpr glm::vec3 FORWARD = glm::vec3(0, 0, -1);
constexpr glm::vec3 BACKWARD = glm::vec3(0, 0, 1);
constexpr glm::vec3 LEFT = glm::vec3(-1, 0, 0);
constexpr glm::vec3 RIGHT = glm::vec3(1, 0, 0);
constexpr glm::vec3 UP = glm::vec3(0, 1, 0);
constexpr glm::vec3 DOWN = glm::vec3(0, -1, 0);

PlayerController::PlayerController(std::shared_ptr<Camera> camera) : m_camera(std::move(camera)) {}

bool PlayerController::ProcessEvent(const SDL_Event &event) {
    if (active && event.type == SDL_MOUSEMOTION) {
        pitch -= static_cast<float>(event.motion.yrel) / 500.0f;
        yaw -= static_cast<float>(event.motion.xrel) / 500.0f;
        pitch = std::min<float>(std::max<float>(-M_PI / 2, pitch), M_PI / 2);
        return true;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN && !active) {
        active = true;
        SDL_SetRelativeMouseMode(SDL_TRUE);
        // todo: snap cursor
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE && active) {
        active = false;
        SDL_SetRelativeMouseMode(SDL_FALSE);
        // SDL_WarpMouseInWindow(sdl_window, width / 2, height / 2);
        // todo: unsnap cursor
    }

    if (event.type == SDL_KEYDOWN) {
        pressed[event.key.keysym.sym] = true;
    }
    if (event.type == SDL_KEYUP) {
        pressed[event.key.keysym.sym] = false;
    }
    return false;
}

void PlayerController::UpdatePosition() {
    auto dt = (float) (m_timer.get_elapsed_seconds());
    transform.translation += (velocity * dt);
    if (active) {
        velocity += transform.rotation * (
                FORWARD * (float) (pressed['w'] || pressed['W']) +
                BACKWARD * (float) (pressed['s'] || pressed['S']) +
                LEFT * (float) (pressed['a'] || pressed['A']) +
                RIGHT * (float) (pressed['d'] || pressed['D']) +
                UP * (float) (pressed[' ']) +
                DOWN * (float) (pressed['z'] || pressed['Z'])
        ) * dt * acceleration_factor;
    }
    velocity *= powf(friction_factor, dt);
    m_timer = timer();

    transform.rotation = (glm::angleAxis(roll, glm::vec3(0, 0, 1)) *
                          glm::angleAxis(yaw, glm::vec3(0, 1, 0)) *
                          glm::angleAxis(pitch, glm::vec3(1, 0, 0)));

    m_camera->SetPosition(transform.translation);
    m_camera->SetRotation(transform.rotation);
}

// todo: test this function!!
/*glm::vec3 direction = glm::normalize(target - transform.translation);
yaw = atan2f(direction.x, direction.z);
float d = sqrtf(direction.x * direction.x + direction.z * direction.z);
pitch = atan2f(direction.y, d);*/

void PlayerController::StartFrameEvent() {
    UpdatePosition();
}

void PlayerController::LookAt(glm::vec3 target) {
    glm::vec3 direction = glm::normalize(target - transform.translation);
    yaw = -atan2f(direction.x, -direction.z);
    float d = sqrtf(direction.x * direction.x + direction.z * direction.z);
    pitch = atan2f(direction.y, d);
}

void PlayerController::SetPosition(glm::vec3 position) {
    transform.translation = position;
}
