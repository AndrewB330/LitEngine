#pragma once

#include <lit/engine/systems/system.hpp>
#include <lit/rendering/opengl/uniform_buffer.hpp>

namespace lit::engine {

    class CameraPreRenderer : public RenderingSystem {
    public:

        CameraPreRenderer(entt::registry& registry);

        void Redraw(double dt) override;

        ~CameraPreRenderer() override = default;

    private:
        std::map<entt::entity, size_t> m_indices;
        std::vector<lit::rendering::opengl::UniformBuffer> m_uniform_buffers;
    };

}