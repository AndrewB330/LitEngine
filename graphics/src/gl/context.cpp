#include <lit/gl/context.hpp>
#include <lit/gl/program.hpp>

using namespace lit::gl;

Context::Context(void *gl_context, std::shared_ptr<spdlog::logger> logger) : m_logger(std::move(logger)) {
    LIT_ASSERT(gl_context != nullptr, "gl_context should not be null");
}

std::shared_ptr<spdlog::logger> Context::GetLogger() const {
    return m_logger;
}
/*
void Context::SetFaceCulling(FaceCulling culling) {
    if (culling == FaceCulling::None) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace((GLenum) culling);
    }
}

void Context::SetDepthFunc(DepthFunc func) {
    if (func == DepthFunc::Disabled) {
        glDisable(GL_DEPTH_TEST);
    } else {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc((GLenum) func);
    }
}

void Context::SetBlendFunc(BlendFunc s_factor, BlendFunc d_factor) {
    if (s_factor == BlendFunc::Disabled || d_factor == BlendFunc::Disabled) {
        glDisable(GL_BLEND);
    } else {
        glEnable(GL_BLEND);
        glBlendFunc((GLenum) s_factor, (GLenum) d_factor);
    }
}*/

ContextObject::ContextObject(std::shared_ptr<Context> ctx) : m_ctx(std::move(ctx)) {
    LIT_ASSERT(m_ctx != nullptr, "ctx should not be null");
}
