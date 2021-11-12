#include <lit/rendering/opengl/api.hpp>
#include <GL/glew.h>
#include <lit/rendering/opengl/frame_buffer.hpp>

using namespace lit::rendering::opengl;

GLenum GetBlendEnum(BlendFunc func) {
    switch (func) {
        case BlendFunc::SrcColor:
            return GL_SRC_COLOR;
        case BlendFunc::OneMinusSrcColor:
            return GL_ONE_MINUS_SRC_COLOR;
        case BlendFunc::DstColor:
            return GL_DST_COLOR;
        case BlendFunc::OneMinusDstColor:
            return GL_ONE_MINUS_DST_COLOR;
        case BlendFunc::SrcAlpha:
            return GL_SRC_ALPHA;
        case BlendFunc::OneMinusSrcAlpha:
            return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFunc::DstAlpha:
            return GL_DST_ALPHA;
        case BlendFunc::OneMinusDstAlpha:
            return GL_ONE_MINUS_DST_ALPHA;
    }
    return GL_ZERO;
}

GLenum GetDepthFuncEnum(DepthFunc func) {
    switch (func) {
        case DepthFunc::Less:
            return GL_LESS;
        case DepthFunc::Greater:
            return GL_GREATER;
        case DepthFunc::Lequal:
            return GL_LEQUAL;
        case DepthFunc::Gequal:
            return GL_GEQUAL;
        case DepthFunc::Always:
            return GL_ALWAYS;
    }
    return 0;
}

GLenum GetFaceCullingEnum(FaceCulling culling) {
    switch (culling) {
        case FaceCulling::Front:
            return GL_FRONT;
        case FaceCulling::Back:
            return GL_BACK;
        case FaceCulling::All:
            return GL_FRONT_AND_BACK;
    }
    return 0;
}

void PublicAPI::SetFaceCulling(FaceCulling culling) {
    glEnable(GL_CULL_FACE);
    glCullFace(GetFaceCullingEnum(culling));
}

void PublicAPI::SetDepthFunc(DepthFunc func) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GetDepthFuncEnum(func));
}

void PublicAPI::SetBlendFunc(BlendFunc s_factor, BlendFunc d_factor) {
    glEnable(GL_BLEND);
    glBlendFunc(GetBlendEnum(s_factor), GetBlendEnum(d_factor));
}

void PublicAPI::DisableFaceCulling() {
    glDisable(GL_CULL_FACE);
}

void PublicAPI::DisableDepthTest() {
    glDisable(GL_DEPTH_TEST);
}

void PublicAPI::DisableBlending() {
    glDisable(GL_BLEND);
}

void PublicAPI::SetViewport(glm::uvec2 viewport) {
    glViewport(0, 0, (int) viewport.x, (int) viewport.y);
}

void PublicAPI::SetViewport(uint32_t width, uint32_t height) {
    glViewport(0, 0, (int) width, (int) height);
}
