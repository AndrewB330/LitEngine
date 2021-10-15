#pragma once
#include <lit/gl/cubemap.hpp>
#include <unordered_map>
#include <memory>
#include <optional>

namespace lit::voxels {

    struct SkyBox {
        std::shared_ptr<lit::gl::Cubemap> cubemap;

        glm::vec3 sky_color = glm::vec3(1);

        bool has_sun = false;
        glm::vec3 sun_direction = glm::vec3(0);
        glm::vec3 sun_color = glm::vec3(0);
    };

    class SkyBoxLoader {
    public:
        SkyBoxLoader() = default;

        SkyBox LoadSkyBox(const std::string & name);

        void SetContext(const std::shared_ptr<gl::Context> & ctx);

    private:
        float GetLuminance(glm::vec3 color);

        float GetLuminance(glm::vec<4, uint8_t> color);

        glm::vec3 GetSkyColor(lit::common::image<uint8_t, 4> & image);

        struct Sun {
            glm::vec3 direction;
            glm::vec3 color;
        };

        std::optional<Sun> GetSun(lit::common::image<uint8_t, 4> & image, int side);

        std::unordered_map<std::string, SkyBox> m_cached;

        std::shared_ptr<gl::Context> m_ctx;
    };

}