#include <lit/gl/cubemap.hpp>

using namespace lit::gl;

Cubemap::~Cubemap() {
    if (m_texture_id) {
        glDeleteTextures(1, &m_texture_id);
    }
}

Cubemap::Cubemap(std::shared_ptr<Context> ctx, const std::array<image_t, 6> &sides) : ContextObject(std::move(ctx)) {
    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture_id);

    for (int i = 0; i < 6; i++) {
        glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                GL_RGBA8,
                (int) sides[i].get_width(), (int) sides[i].get_height(),
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                sides[i].data()
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemap::Bind(int texture_index) {
    if (m_texture_id) {
        glActiveTexture(GL_TEXTURE0 + texture_index);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture_id);
    }
}
