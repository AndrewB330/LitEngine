#pragma once
#include <optional>

#include "assert.hpp"
#include <spdlog/spdlog.h>
#include <glm/vec2.hpp>

#define TRACE(msg, ...) m_ctx.GetLogger().trace(msg, __VA_ARGS__);

namespace lit::rendering::opengl {

    enum class FaceCulling {
        Front,
        Back,
        All
    };

    enum class DepthFunc {
        Less,
        Greater,
        Lequal,
        Gequal,
        Always
    };

    enum class BlendFunc {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha
    };

    class PublicAPI {
    public:

        void SetFaceCulling(FaceCulling culling);

        void SetDepthFunc(DepthFunc func);

        void SetBlendFunc(BlendFunc s_factor, BlendFunc d_factor);

        void DisableFaceCulling();

        void DisableDepthTest();

        void DisableBlending();

        void SetViewport(glm::uvec2 viewport);

        void SetViewport(uint32_t width, uint32_t height);
    };

}