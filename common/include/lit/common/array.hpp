#pragma once

#include <vector>

namespace lit::common {
    template<typename T>
    class Array2D {
    public:
        Array2D(size_t width, size_t height) : m_data(width * height), m_width(width), m_height(height) {}

        const T& at(size_t x, size_t y) const {
            return m_data.at(x * m_height + y);
        }

        T& at(size_t x, size_t y) {
            return m_data.at(x * m_height + y);
        }

        size_t GetWidth() const {
            return m_width;
        }

        size_t GetHeight() const {
            return m_height;
        }

    private:
        std::vector<T> m_data;
        size_t m_width;
        size_t m_height;
    };
}