#pragma once

#include <memory>
#include <vector>
#include "texture.hpp"

namespace lit::rendering::opengl {

    using Attachment = TextureInternalFormat;

    struct FrameBufferInfo {
        uint32_t width = 512;
        uint32_t height = 512;

        std::vector<Attachment> attachments;
    };

    enum class BindType {
        Read,
        Draw,
        Both
    };

    class FrameBuffer {
    public:
        static FrameBuffer Create(const FrameBufferInfo &info);

        static FrameBuffer Default();

        FrameBuffer(FrameBuffer &&other) noexcept = default;

        FrameBuffer &operator=(FrameBuffer &&other) noexcept = default;

        ~FrameBuffer();

        void Bind(BindType type = BindType::Both) const;

        void Unbind() const;

        std::vector<std::weak_ptr<Texture2D>> GetAttachmentTextures() const;

        uint32_t GetWidth() const;

        uint32_t GetHeight() const;

        glm::uvec2 GetViewport() const;

        void BlitTo(FrameBuffer &dest) const;

        void BlitToDefault() const;

        bool IsDefault();

    private:
        FrameBuffer(const FrameBufferInfo &info);

        FrameBuffer() = default;

        FrameBufferInfo m_info;
        std::unique_ptr<uint32_t> m_frame_buffer_id;
        bool m_has_depth_attachment = false;
        std::vector<std::shared_ptr<Texture2D>> m_attachments;
    };

}
