#pragma once

#include <memory>
#include <vector>
#include "texture.hpp"

namespace lit::gl {

    enum class Attachment {
        DepthComponent = (int) TextureInternalFormat::DepthComponent,
        R16 = (int) TextureInternalFormat::R16F,
        RGB = (int) TextureInternalFormat::RGB,
        RGBA = (int) TextureInternalFormat::RGBA,
        RGB16 = (int) TextureInternalFormat::RGB16F,
        RGB32 = (int) TextureInternalFormat::RGB32F,
        RGBA32 = (int) TextureInternalFormat::RGBA32F,
        RGB32I = (int) TextureInternalFormat::RGB32I,
        RGBA32I = (int) TextureInternalFormat::RGBA32I,
        RGB32UI = (int) TextureInternalFormat::RGB32UI,
        RGBA32UI = (int) TextureInternalFormat::RGBA32UI,
    };

    struct FrameBufferInfo {
        int width = kDefaultTexSize;
        int height = kDefaultTexSize;

        std::vector<Attachment> attachments;
    };

    class FrameBuffer : public ContextObject {
    public:
        FrameBuffer(const std::shared_ptr<Context>& ctx, const FrameBufferInfo &info);

        ~FrameBuffer();

        void Bind();

        std::vector<std::weak_ptr<Texture2D>> GetAttachments();

        int GetWidth() const;

        int GetHeight() const;

    private:
        uint32_t m_frame_buffer_id = 0;
        FrameBufferInfo m_info;
        bool m_has_depth_attachment = false;
        std::vector<std::shared_ptr<Texture2D>> m_attachments;
    };

}
