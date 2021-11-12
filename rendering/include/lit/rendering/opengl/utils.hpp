#pragma once
#include <GL/glew.h>
#include <spdlog/spdlog.h>

#define GL_CALL(x) x; GlLogCall(#x, __FILE__, __LINE__);

static void GlClearError() {
    while(glGetError() != GL_NO_ERROR);
}

static bool GlLogCall(const char * name, const char * file, int line) {
    static GLenum prev_err = 0;
    while(GLenum err = glGetError()) {
        if (err != prev_err) {
            spdlog::default_logger()->error("OpenGL {}, {}:{}", err, file, line);
        }
        prev_err = err;
        //__debugbreak();
        return false;
    }
    prev_err = 0;
    return true;
}