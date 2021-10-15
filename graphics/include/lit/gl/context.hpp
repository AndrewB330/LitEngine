#pragma once

#include <GL/glew.h>
#include <memory>
#include "assert.hpp"

namespace lit::gl {

    const uint32_t kDefaultTexSize = 512;

    class Context {
    public:
        explicit Context(void *gl_context, std::shared_ptr<spdlog::logger> logger);

        std::shared_ptr<spdlog::logger> GetLogger() const;

    private:
        std::shared_ptr<spdlog::logger> m_logger;
    };

    // Base class for any object that needs gl context
    class ContextObject {
    public:
        explicit ContextObject(std::shared_ptr<Context> ctx);

        ContextObject(const ContextObject &) = delete;

    protected:
        std::shared_ptr<Context> m_ctx;
    };
}