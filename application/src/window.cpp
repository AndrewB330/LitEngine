#include <utility>
#include <lit/application/window.hpp>
#include <GL/glew.h>

using namespace lit::application;

Window::Window(spdlog::logger_ptr logger, WindowInfo info): m_info(std::move(info)), m_logger(std::move(logger)) {}

Window::~Window() {
    if (sdl_window != nullptr) {
        SDL_DestroyWindow(sdl_window);
        m_logger->info("Window \"{}\" destroyed", m_info.title.c_str());
    }
}

bool Window::Init() {
    if (initialized) {
        return true;
    }

    // OpenGL 4.5 todo: make it as a parameter
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    sdl_window = SDL_CreateWindow(m_info.title.c_str(),
                                  SDL_WINDOWPOS_CENTERED_MASK,
                                  SDL_WINDOWPOS_CENTERED_MASK,
                                  m_info.width,
                                  m_info.height,
                                  SDL_WINDOW_OPENGL |
                                  SDL_WINDOW_SHOWN |
                                  (m_info.resizable ? SDL_WINDOW_RESIZABLE : 0) |
                                  (m_info.maximized ? SDL_WINDOW_MAXIMIZED : 0));
    if (sdl_window == nullptr) {
        m_logger->error("SDL_CreateWindow: {}", SDL_GetError());
        return false;
    } else {
        m_logger->info("[1/3] SDL Window \"{}\" created.", m_info.title.c_str());
    }
    gl_context = SDL_GL_CreateContext(sdl_window);
    if (gl_context == nullptr) {
        m_logger->error("SDL_GL_CreateContext: {}", SDL_GetError());
        return false;
    } else {
        m_logger->info("[2/3] SDL GL Context Created.");
    }
    glewInit();
    m_logger->info("[3/3] GLEW Initialized.");
    SDL_GL_SetSwapInterval(0);
    glClearColor(0, 0, 0, 0);
    return initialized = true;
}

bool Window::ProcessEvent(const SDL_Event &event) {
    if (!initialized || closed) {
        return false;
    }
    auto my_id = SDL_GetWindowID(sdl_window);

    if (event.type == SDL_WINDOWEVENT && event.window.windowID == my_id) {
        if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
            closed = true;
            m_logger->info("Window \"{}\" was closed by user", m_info.title.c_str());
        }
        return true;
    }

    for (auto &l : listeners) {
        if (l->ProcessEvent(event)) {
            return true;
        }
    }

    return false;
}

Window::Window(Window &&window) noexcept {
    std::swap(m_info, window.m_info);
    std::swap(sdl_window, window.sdl_window);
    std::swap(gl_context, window.gl_context);
    std::swap(initialized, window.initialized);
    std::swap(closed, window.closed);
    std::swap(renderers, window.renderers);
    std::swap(listeners, window.listeners);
}

bool Window::IsClosed() const {
    return closed;
}

void Window::Redraw() {
    if (!initialized || closed) {
        return;
    }

    for (auto &l : listeners) {
        l->StartFrameEvent();
    }

    glClear(GL_COLOR_BUFFER_BIT);
    for (auto &r : renderers) {
        r->Redraw();
    }

    SDL_GL_SwapWindow(sdl_window);
}

void Window::AddRenderer(std::shared_ptr<WindowRenderer> renderer) {
    if (!initialized || closed) {
        return;
    }
    renderer->m_sdl_window = sdl_window;
    renderer->m_context = gl_context;
    renderer->Init();
    renderers.push_back(std::move(renderer));
}

void Window::AddListener(std::shared_ptr<WindowListener> listener) {
    if (!initialized || closed) {
        return;
    }
    listeners.push_back(std::move(listener));
}
