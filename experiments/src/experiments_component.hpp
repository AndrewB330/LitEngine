#pragma once

#define GLM_FORCE_SWIZZLE

#include <lit/application/window_renderer.hpp>
#include <memory>
#include "pipeline.hpp"
#include "model.hpp"

using lit::application::WindowRenderer;

class ExperimentsComponent : public WindowRenderer {
public:
    explicit ExperimentsComponent(std::shared_ptr<lit::voxels::Camera> camera);

    ~ExperimentsComponent() override = default;

    bool Init() override;

    void Redraw() override;

private:

    void InitPipeline();

    void InitWorld();

    glm::vec2 GetViewport() const;

    SDL_Window *sdl_window = nullptr;

    bool m_old_active = false;

    std::shared_ptr<lit::gl::Context> m_ctx;

    std::shared_ptr<lit::voxels::Camera> m_camera;
    std::unique_ptr<Pipeline> m_pipeline;
    std::unique_ptr<World> m_world;
};
