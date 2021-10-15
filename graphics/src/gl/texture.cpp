#include <lit/gl/texture.hpp>

using namespace lit::gl;

Texture2D::Texture2D(std::shared_ptr<Context> ctx, const Texture2DInfo &textureInfo): ContextObject(std::move(ctx)) {
    m_info = textureInfo;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int) m_info.min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int) m_info.mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int) m_info.wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int) m_info.wrap_t);
    glTexImage2D(GL_TEXTURE_2D, 0, (int) m_info.internal_format, m_info.width, m_info.height, 0,
                 (int) m_info.data_format,
                 (int) m_info.data_type, nullptr);
    for (int i = 1; i <= m_info.mip_levels; i++) {
        glTexImage2D(GL_TEXTURE_2D, i, (int) m_info.internal_format, m_info.width >> i, m_info.height >> i, 0,
                     (int) m_info.data_format,
                     (int) m_info.data_type, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2D::~Texture2D() {
    if (m_texture_id) {
        glDeleteTextures(1, &m_texture_id);
    }
}

void Texture2D::Bind(int texture_index) {
    if (m_texture_id) {
        glActiveTexture(GL_TEXTURE0 + texture_index);
        glBindTexture(GL_TEXTURE_2D, m_texture_id);
    }
}

Texture3D::Texture3D(std::shared_ptr<Context> ctx, const Texture3DInfo &textureInfo): ContextObject(std::move(ctx)) {
    m_info = textureInfo;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_3D, m_texture_id);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, (int) m_info.min_filter);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, (int) m_info.mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int) m_info.wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int) m_info.wrap_t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, (int) m_info.wrap_r);

    glTexImage3D(GL_TEXTURE_3D, 0, (int) m_info.internal_format, m_info.width, m_info.height, m_info.depth, 0,
                 (int) m_info.data_format,
                 (int) m_info.data_type, nullptr);
    for (int i = 1; i <= m_info.mip_levels; i++) {
        glTexImage3D(GL_TEXTURE_3D, i, (int) m_info.internal_format,
                     std::max(kMinSide, m_info.width >> i),
                     std::max(kMinSide, m_info.height >> i),
                     std::max(kMinSide, m_info.depth >> i), 0,
                     (int) m_info.data_format,
                     (int) m_info.data_type, nullptr);
    }
    glBindTexture(GL_TEXTURE_3D, 0);
}

Texture3D::~Texture3D() {
    if (m_texture_id) {
        glDeleteTextures(1, &m_texture_id);
    }
}

void Texture3D::Bind(int texture_index) {
    if (m_texture_id) {
        glActiveTexture(GL_TEXTURE0 + texture_index);
        glBindTexture(GL_TEXTURE_3D, m_texture_id);
    }
}

uint8_t lit::gl::GetChannelsNumber(TextureDataFormat format) {
    switch (format) {
        case TextureDataFormat::RedInteger:
        case TextureDataFormat::Red:
        case TextureDataFormat::DepthComponent:
            return 1;
        case TextureDataFormat::RGB:
            return 3;
        case TextureDataFormat::RGBA:
        case TextureDataFormat::RGBAInteger:
            return 4;
    }
    return 0;
}
