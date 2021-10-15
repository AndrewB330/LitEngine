#include <algorithm>
#include <lit/gl/frame_buffer.hpp>

using namespace lit::gl;

FrameBuffer::~FrameBuffer() {
    if (m_frame_buffer_id) {
        m_attachments.clear();
        glDeleteFramebuffers(1, &m_frame_buffer_id);
    }
}

FrameBuffer::FrameBuffer(const std::shared_ptr<Context>& ctx, const FrameBufferInfo &info): ContextObject(ctx) {
    m_info = info;

    glGenFramebuffers(1, &m_frame_buffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_id);

    int color_attachment_index = 0;
    for (auto &attachment : m_info.attachments) {
        bool is_depth = (attachment == Attachment::DepthComponent);

        Texture2DInfo texture_info;
        texture_info.width = m_info.width;
        texture_info.height = m_info.height;
        texture_info.internal_format = (TextureInternalFormat) attachment;

        if (is_depth) {
            texture_info.data_format = TextureDataFormat::DepthComponent;
        }

        m_attachments.push_back(std::make_shared<Texture2D>(ctx, texture_info));

        if (!is_depth) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attachment_index, GL_TEXTURE_2D,
                                   m_attachments.back()->m_texture_id, 0);
            color_attachment_index++;
        } else {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                                   m_attachments.back()->m_texture_id, 0);
            m_has_depth_attachment = true;
        }
    }

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    //LIT_THROW_IF(status != GL_FRAMEBUFFER_COMPLETE, "FrameBuffer is not complete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Bind() {
    if (m_frame_buffer_id) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_id);
        if (m_has_depth_attachment) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
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

            glDrawBuffers((int) num, color_attachments);

            delete[] color_attachments;
        }

        if (m_has_depth_attachment) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        } else {
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }
}

std::vector<std::weak_ptr<Texture2D>> FrameBuffer::GetAttachments() {
    return std::vector<std::weak_ptr<Texture2D>>(m_attachments.begin(), m_attachments.end());
}

int FrameBuffer::GetWidth() const {
    return m_info.width;
}

int FrameBuffer::GetHeight() const {
    return m_info.height;
}
