#include <algorithm>
#include <lit/rendering/opengl/frame_buffer.hpp>
#include <GL/glew.h>
#include <spdlog/spdlog.h>
//#include <lit/application/assert.hpp>
#include <lit/rendering/opengl/assert.hpp>
#include <lit/rendering/opengl/utils.hpp>

using namespace lit::rendering::opengl;

FrameBuffer::~FrameBuffer() {
    if (m_frame_buffer_id) {
        m_attachments.clear();
        GL_CALL(glDeleteFramebuffers(1, m_frame_buffer_id.get()));
        m_frame_buffer_id.reset();
        spdlog::default_logger()->trace("FrameBuffer destroyed");
    }
}

FrameBuffer::FrameBuffer(const FrameBufferInfo &info) {
    m_info = info;

    m_frame_buffer_id = std::make_unique<uint32_t>();
    GL_CALL(glGenFramebuffers(1, m_frame_buffer_id.get()));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, *m_frame_buffer_id));

    int color_attachment_index = 0;
    for (auto &attachment: m_info.attachments) {
        bool is_depth = (attachment == Attachment::DepthComponent);

        Texture2DInfo texture_info;
        texture_info.width = m_info.width;
        texture_info.height = m_info.height;
        texture_info.internal_format = (TextureInternalFormat) attachment;

        if (is_depth) {
            texture_info.data_format = TextureDataFormat::DepthComponent;
        }

        m_attachments.push_back(std::shared_ptr<Texture2D>(new Texture2D(texture_info)));

        if (!is_depth) {
            GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attachment_index, GL_TEXTURE_2D,
                                           *m_attachments.back()->m_texture_id, 0));
            color_attachment_index++;
        } else {
            GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                                           *m_attachments.back()->m_texture_id, 0));
            m_has_depth_attachment = true;
        }
    }

    GL_CALL(auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

    LIT_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "FrameBuffer is not complete", spdlog::default_logger());

    spdlog::default_logger()->trace("FrameBuffer created");

    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::Bind(BindType type) const {
    if (m_frame_buffer_id) {
        switch (type) {
            case BindType::Read:
                GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, *m_frame_buffer_id));
            case BindType::Draw:
            GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, *m_frame_buffer_id));
            case BindType::Both:
            GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, *m_frame_buffer_id));
        }
        if (m_has_depth_attachment) {
            GL_CALL(glEnable(GL_DEPTH_TEST));
        } else {
            GL_CALL(glDisable(GL_DEPTH_TEST));
        }
        if (!m_attachments.empty()) {
            size_t num = m_attachments.size() - m_has_depth_attachment;
            auto *color_attachments = new GLenum[num];

            for (size_t i = 0, j = 0; i < m_attachments.size(); i++) {
                if (m_info.attachments[i] != Attachment::DepthComponent) {
                    color_attachments[j] = GL_COLOR_ATTACHMENT0 + j;
                    j++;
                }
            }

            GL_CALL(glDrawBuffers((int) num, color_attachments));

            delete[] color_attachments;
        }

        if (m_has_depth_attachment) {
            GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        } else {
            GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
        }
    }
}

void FrameBuffer::Unbind() const {
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

std::vector<std::weak_ptr<Texture2D>> FrameBuffer::GetAttachmentTextures() const {
    return {m_attachments.begin(), m_attachments.end()};
}

uint32_t FrameBuffer::GetWidth() const {
    return m_info.width;
}

uint32_t FrameBuffer::GetHeight() const {
    return m_info.height;
}

FrameBuffer FrameBuffer::Default() {
    return {};
}

bool FrameBuffer::IsDefault() {
    return !m_frame_buffer_id;
}

glm::uvec2 FrameBuffer::GetViewport() const {
    return {m_info.width, m_info.height};
}

void FrameBuffer::BlitTo(FrameBuffer &dest) const {
    if (!m_frame_buffer_id)
        return;

    if (dest.GetViewport() != GetViewport()) {
        spdlog::default_logger()->warn("Framebuffer dims don't not match for blit: {}x{} != {}x{}",
                                       GetWidth(), GetHeight(), dest.GetWidth(), dest.GetHeight());
    }

    dest.Bind(BindType::Draw);
    Bind(BindType::Read);
    GL_CALL(glBlitFramebuffer(0, 0, GetWidth(), GetHeight(), 0, 0, dest.GetWidth(), dest.GetHeight(),
                      GL_COLOR_BUFFER_BIT, GL_NEAREST));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::BlitToDefault() const {
    if (!m_frame_buffer_id)
        return;

    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, *m_frame_buffer_id));
    GL_CALL(glBlitFramebuffer(0, 0, GetWidth(), GetHeight(), 0, 0, GetWidth(), GetHeight(),
                              GL_COLOR_BUFFER_BIT, GL_NEAREST));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

FrameBuffer FrameBuffer::Create(const FrameBufferInfo &info) {
    return {info};
}
