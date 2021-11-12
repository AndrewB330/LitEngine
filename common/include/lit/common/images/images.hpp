#pragma once

#define GLM_FORCE_SWIZZLE

#include <algorithm>
#include <vector>
#include <string>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace lit::common {

    /**
     * Represents 2-dimensional image. Pixel type is compatible with glm::vec
     * @tparam T - underlying type.
     * @tparam C - number of channels (values per pixel)
     */
    template<typename T, uint8_t C = 3>
    class Image {
    public:
        using pixel_t = glm::vec<C, T>;

        Image(uint32_t width, uint32_t height) : m_width(width), m_height(height) {
            m_data.assign(width * height * C, T());
        }

        Image() = default;

        pixel_t GetPixel(uint32_t row, uint32_t col) const {
            glm::vec<C, T> pixel;
            auto index = GetIndex(row, col);
            std::copy(m_data.begin() + index, m_data.begin() + index + C, &pixel[0]);
            return pixel;
        }

        void Fill(const pixel_t &value) {
            auto it = (pixel_t *) m_data.data();
            std::fill(it, it + m_width * m_height, value);
        }

        void SetPixel(uint32_t row, uint32_t col, const pixel_t &value) {
            std::copy(&value[0], &value[0] + C, m_data.begin() + GetIndex(row, col));
        }

        uint32_t GetWidth() const {
            return m_width;
        }

        uint32_t GetHeight() const {
            return m_height;
        }

        glm::ivec2 GetDimensions() const {
            return {m_width, m_height};
        }

        const T *GetDataPointer() const {
            return m_data.data();
        }

    private:
        inline uint32_t GetIndex(uint32_t row, uint32_t col) const {
            return C * (row + col * m_width);
        }

        uint32_t m_width = 0;
        uint32_t m_height = 0;

        std::vector<T> m_data = {};
    };

    /**
     * Represents 3-dimensional image with one value per pixel
     * @tparam T - underlying type
     */
    template<typename T>
    class Image3D {
    public:
        using pixel_t = T;

        Image3D(uint32_t width, uint32_t height, uint32_t depth, T fill_value = T())
                : m_width(width), m_height(height), m_depth(depth), m_data(width * height * depth, fill_value) {}

        Image3D() = default;

        inline pixel_t GetPixel(uint32_t x, uint32_t y, uint32_t z) const {
            return m_data[GetIndex(x, y, z)];
        }

        inline pixel_t &GetPixel(uint32_t x, uint32_t y, uint32_t z) {
            return m_data[GetIndex(x, y, z)];
        }

        inline pixel_t GetPixel(const glm::ivec3 &pos) const {
            return m_data[GetIndex(pos.x, pos.y, pos.z)];
        }

        inline pixel_t &GetPixel(const glm::ivec3 &pos) {
            return m_data[GetIndex(pos.x, pos.y, pos.z)];
        }

        inline void SetPixel(uint32_t x, uint32_t y, uint32_t z, const pixel_t &value) {
            m_data[GetIndex(x, y, z)] = value;
        }

        inline void SetPixel(const glm::ivec3 &pos, const pixel_t &value) {
            m_data[GetIndex(pos.x, pos.y, pos.z)] = value;
        }

        inline void Fill(const pixel_t &value) {
            std::fill(m_data.begin(), m_data.end(), value);
        }

        inline uint32_t GetWidth() const {
            return m_width;
        }

        inline uint32_t GetHeight() const {
            return m_height;
        }

        inline uint32_t GetDepth() const {
            return m_depth;
        }

        inline glm::ivec3 GetDimensions() const {
            return {m_width, m_height, m_depth};
        }

        inline const T *GetDataPointer() const {
            return m_data.data();
        }

    private:
        inline uint32_t GetIndex(uint32_t x, uint32_t y, uint32_t z) const {
            return x + y * m_width + z * m_width * m_height;
        }

        uint32_t m_width = 0;
        uint32_t m_height = 0;
        uint32_t m_depth = 0;

        std::vector<T> m_data = {};
    };

    Image<uint8_t, 4> ReadPNG_RGBA(const std::string &filename);

    Image<uint8_t, 3> ReadPNG_RGB(const std::string &filename);

}
