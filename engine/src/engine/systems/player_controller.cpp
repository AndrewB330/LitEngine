#include <lit/engine/systems/observer_input_controller.hpp>
#include <lit/engine/components/transform.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace lit::engine;

constexpr glm::dvec3 FORWARD = glm::dvec3(0, 0, 1);
constexpr glm::dvec3 BACKWARD = glm::dvec3(0, 0, -1);
constexpr glm::dvec3 LEFT = glm::dvec3(-1, 0, 0);
constexpr glm::dvec3 RIGHT = glm::dvec3(1, 0, 0);
constexpr glm::dvec3 UP = glm::dvec3(0, 1, 0);
constexpr glm::dvec3 DOWN = glm::dvec3(0, -1, 0);

ObserverInputController::ObserverInputController(entt::registry & registry, entt::entity player) : System(registry), m_player(player) {}

bool ObserverInputController::ProcessInput(const UserInput &input) {
    auto event = input.event;
    if (m_active && event.type == SDL_MOUSEMOTION) {
        m_pitch += static_cast<double>(event.motion.yrel) / 500.0f;
        m_yaw += static_cast<double>(event.motion.xrel) / 500.0f;
        m_pitch = std::min<double>(std::max<double>(-glm::pi<double>() / 2.0, m_pitch), glm::pi<double>() / 2.0);
        return true;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN && !m_active) {
        m_active = true;
        SDL_SetRelativeMouseMode(SDL_TRUE);
        // todo: snap cursor
    }
    if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == 'e') && m_active) {
        m_active = false;
        SDL_SetRelativeMouseMode(SDL_FALSE);
        // SDL_WarpMouseInWindow(sdl_window, width / 2, height / 2);
        // todo: unsnap cursor
    }

    if (event.type == SDL_KEYDOWN) {
        m_pressed[event.key.keysym.sym] = true;
    }
    if (event.type == SDL_KEYUP) {
        m_pressed[event.key.keysym.sym] = false;
    }
    return false;
}

void ObserverInputController::Update(double dt) {
    auto &transform = m_registry.get<Transform3d>(m_player);

    transform.translation += (m_velocity * dt);

    if (m_active) {
        m_velocity += transform.rotation * (
                FORWARD * (double) (m_pressed['w'] || m_pressed['W']) +
                BACKWARD * (double) (m_pressed['s'] || m_pressed['S']) +
                LEFT * (double) (m_pressed['a'] || m_pressed['A']) +
                RIGHT * (double) (m_pressed['d'] || m_pressed['D']) +
                UP * (double) (m_pressed[' ']) +
                DOWN * (double) (m_pressed['z'] || m_pressed['Z'])
        ) * dt * m_acceleration_factor;
    }

    m_velocity *= pow(m_friction_factor, dt);

    transform.rotation = (glm::angleAxis(m_roll, glm::dvec3(0, 0, 1)) *
                          glm::angleAxis(m_yaw, glm::dvec3(0, 1, 0)) *
                          glm::angleAxis(m_pitch, glm::dvec3(1, 0, 0)));
}
