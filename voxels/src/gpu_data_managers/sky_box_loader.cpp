#include "sky_box_loader.hpp"
#include <lit/common/images/image.hpp>
#include <algorithm>
#include <vector>
#include <glm/glm.hpp>
//#include <lit/common/logging.hpp>

using namespace lit::voxels;

SkyBox lit::voxels::SkyBoxLoader::LoadSkyBox(const std::string &name) {
    if (m_cached.find(name) != m_cached.end()) {
        return m_cached[name];
    }

    std::string prefix = "../assets/sky_boxes/" + name + '/';
    SkyBox res;
    std::array<gl::Cubemap::image_t, 6> sides;
    std::vector<std::string> filenames = {"right.png",
                                          "left.png",
                                          "up.png",
                                          "down.png",
                                          "back.png",
                                          "front.png"};
    std::optional<SkyBoxLoader::Sun> best_sun;
    for (int i = 0; i < 6; i++) {
        sides[i] = common::read_png(prefix + filenames[i]);
        auto sun = GetSun(sides[i], i);
        if (!best_sun || (sun && GetLuminance(best_sun->color) < GetLuminance(sun->color))) {
            best_sun = sun;
        }
    }
    res.sky_color = GetSkyColor(sides[2]);
    res.cubemap = std::make_shared<gl::Cubemap>(m_ctx, sides);
    res.has_sun = best_sun.has_value();
    if (res.has_sun) {
        res.sun_color = best_sun->color;
        res.sun_direction = best_sun->direction;
        //lit::common::Logger::LogInfo("SkyBox sun color: %f, %f, %f", res.sun_color.x, res.sun_color.y, res.sun_color.z);
    }
    return m_cached[name] = res;
}

glm::vec3 lit::voxels::SkyBoxLoader::GetSkyColor(lit::common::image<uint8_t, 4> &image) {
    using pixel_t = lit::common::image<uint8_t, 4>::pixel_t;
    std::vector<pixel_t> pixels;
    pixels.reserve(image.get_width() * image.get_height());
    for (int i = 0; i < image.get_width(); i++) {
        for (int j = 0; j < image.get_height(); j++) {
            pixels.push_back(image.get_pixel(i, j));
        }
    }
    std::sort(pixels.begin(), pixels.end(), [&](auto a, auto b) {
        return GetLuminance(a) > GetLuminance(b);
    });
    glm::vec3 sum = glm::vec3();
    int cnt = (int) (pixels.size() * 0.2 + 1);
    for (int i = 0; i < cnt; i++) sum += glm::vec3(pixels[i]) / 255.0f;
    return sum / float(cnt);
}

std::optional<SkyBoxLoader::Sun> lit::voxels::SkyBoxLoader::GetSun(lit::common::image<uint8_t, 4> &image, int side) {
    using pixel_t = lit::common::image<uint8_t, 4>::pixel_t;

    glm::vec2 sum_pos = glm::vec2();
    float sum_luminance = 0.0f;

    for (int i = 0; i < image.get_width(); i++) {
        for (int j = 0; j < image.get_height(); j++) {
            float luminance = GetLuminance(image.get_pixel(i, j));
            sum_luminance += powf(luminance, 10);
            sum_pos += glm::vec2(i, j) * powf(luminance, 10);
        }
    }

    glm::ivec2 pos = glm::ivec2(sum_pos / sum_luminance);

    const int radius = 8;

    glm::vec3 sum = glm::vec3();
    int cnt = 0;
    for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
            int x = pos.x + dx;
            int y = pos.y + dy;
            if (x < 0 || y < 0 || x >= image.get_width() || y>= image.get_height()) continue;
            cnt++;
            sum += glm::vec3(image.get_pixel(x, y)) / 255.0f;
        }
    }

    Sun res;
    res.color = sum / float(cnt);
    float w = image.get_width();
    float h = image.get_height();
    glm::vec2 posf = glm::vec2(pos);
    switch(side) {
        case 5:
            res.direction = glm::vec3((w * 0.5f - posf.x) / w * 2, (h * 0.5f - posf.y) / h * 2, -1); // front
            break;
        default:
            return std::nullopt;
    }
    res.direction = glm::normalize(res.direction + glm::vec3(0, 0.3, 0));
    return res;
}

float SkyBoxLoader::GetLuminance(glm::vec<4, uint8_t> color) {
    return (color.x + color.y + color.z) / (255.0f * 3.0f);
}

float SkyBoxLoader::GetLuminance(glm::vec3 color) {
    return (color.x + color.y + color.z) / 3.0f;
}

void SkyBoxLoader::SetContext(const std::shared_ptr<gl::Context> &ctx) {
    m_ctx = ctx;
}
