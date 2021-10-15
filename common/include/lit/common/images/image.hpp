#pragma once

#define GLM_FORCE_SWIZZLE

#include <algorithm>
#include <vector>
#include <string>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace lit::common {

    template<typename T, uint8_t C = 3>
    class image {
    public:
        using pixel_t = glm::vec<C, T>;

        image(uint32_t width, uint32_t height) : m_width(width), m_height(height) {
            m_data.assign(width * height * C, T());
        }

        image() = default;

        pixel_t get_pixel(uint32_t row, uint32_t col) const {
            glm::vec<C, T> pixel;
            auto index = get_index(row, col);
            std::copy(m_data.begin() + index, m_data.begin() + index + C, &pixel[0]);
            return pixel;
        }

        void fill(const pixel_t &value) {
            auto it = (pixel_t *) m_data.data();
            std::fill(it, it + m_width * m_height, value);
        }

        void set_pixel(uint32_t row, uint32_t col, const pixel_t &value) {
            std::copy(&value[0], &value[0] + C, m_data.begin() + get_index(row, col));
        }

        uint32_t get_width() const {
            return m_width;
        }

        uint32_t get_height() const {
            return m_height;
        }

        glm::ivec2 get_dims() const {
            return glm::ivec2(m_width, m_height);
        }

        const T *data() const {
            return m_data.data();
        }

    private:
        uint32_t get_index(uint32_t row, uint32_t col) const {
            return C * (row + col * m_width);
        }

        uint32_t m_width = 0;
        uint32_t m_height = 0;

        std::vector<T> m_data = {};
    };

    template<typename T>
    class image3d {
    public:
        using pixel_t = T;

        image3d(uint32_t width, uint32_t height, uint32_t depth, T fill_value = T())
                : m_width(width), m_height(height), m_depth(depth), m_data(width * height * depth, fill_value) {}

        image3d() = default;

        pixel_t get_pixel(uint32_t x, uint32_t y, uint32_t z) const {
            return m_data[get_index(x, y, z)];
        }

        pixel_t &get_pixel(uint32_t x, uint32_t y, uint32_t z) {
            return m_data[get_index(x, y, z)];
        }

        pixel_t get_pixel(const glm::ivec3 &pos) const {
            return m_data[get_index(pos.x, pos.y, pos.z)];
        }

        pixel_t &get_pixel(const glm::ivec3 &pos) {
            return m_data[get_index(pos.x, pos.y, pos.z)];
        }

        void set_pixel(uint32_t x, uint32_t y, uint32_t z, const pixel_t &value) {
            m_data[get_index(x, y, z)] = value;
        }

        void set_pixel(const glm::ivec3 &pos, const pixel_t &value) {
            m_data[get_index(pos.x, pos.y, pos.z)] = value;
        }

        void fill(const pixel_t &value) {
            std::fill(m_data.begin(), m_data.end(), value);
        }

        uint32_t get_width() const {
            return m_width;
        }

        uint32_t get_height() const {
            return m_height;
        }

        uint32_t get_depth() const {
            return m_depth;
        }

        glm::ivec3 get_dims() const {
            return glm::ivec3(m_width, m_height, m_depth);
        }

        const T *data() const {
            return m_data.data();
        }

    private:
        uint32_t get_index(uint32_t x, uint32_t y, uint32_t z) const {
            return x + y * m_width + z * m_width * m_height;
        }

        uint32_t m_width = 0;
        uint32_t m_height = 0;
        uint32_t m_depth = 0;

        std::vector<T> m_data = {};
    };

    image<uint8_t, 4> read_png(const std::string &filename);

    image<uint8_t, 3> read_png_rgb(const std::string &filename);

}
