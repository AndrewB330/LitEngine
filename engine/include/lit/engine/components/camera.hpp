#pragma once

#include <lit/engine/components/transform.hpp>
#include <lit/rendering/opengl/frame_buffer.hpp>
#include <lit/rendering/opengl/uniform_buffer.hpp>
#include <glm/vec2.hpp>

namespace lit::engine {

    class CameraComponent {
    public:
        using OpenglFrameBuffer = lit::rendering::opengl::FrameBuffer;

        explicit CameraComponent(glm::uvec2 viewport);

        const OpenglFrameBuffer &GetFrameBuffer() const;

        glm::dmat4 GetProjectionMatrix() const;

        glm::uvec2 GetViewport() const;

        void SetViewport(glm::uvec2 viewport);

    private:
        OpenglFrameBuffer m_frame_buffer;

        double m_z_near = 0.01f;
        double m_z_far = 100.0f;
        double m_field_of_view = 90.0f;
    };

}
