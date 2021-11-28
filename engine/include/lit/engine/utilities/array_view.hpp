#pragma once

#include <glm/vec3.hpp>

namespace lit::engine {

    /// <summary>
    /// Allows to access some contiguous data as a 3D array.
    /// </summary>
    /// <typeparam name="T">Type of element</typeparam>
    template<typename T>
    class Array3DView {
    public:
        Array3DView(size_t width, size_t height, size_t depth, T* begin)
            :m_width(width), m_height(height), m_depth(depth), m_begin(begin) {}

        Array3DView(glm::ivec3 dims, T* begin)
            :Array3DView(dims.x, dims.y, dims.z, begin) {}

        T& At(size_t i, size_t j, size_t k) {
            return *(m_begin + (i * m_height * m_depth + j * m_depth + k));
        }

        T& At(glm::ivec3 pos) {
            return At(pos.x, pos.y, pos.z);
        }

        const T& At(size_t i, size_t j, size_t k) const {
            return *(m_begin + (i * m_height * m_depth + j * m_depth + k));
        }

        const T& At(glm::ivec3 pos) const {
            return At(pos.x, pos.y, pos.z);
        }

        void CopyTo(Array3DView<T>& other) const {
            memcpy(other.m_begin, m_begin, m_width * m_height * m_depth * sizeof(T));
            return *this;
        }

        void Fill(T value) {
            if (value == 0) {
                memset(m_begin, 0, m_width * m_height * m_depth * sizeof(T));
            }
            else {
                std::fill(m_begin, m_begin + m_width * m_height * m_depth, value);
            }
        }

        const T* Data() const {
            return m_begin;
        }

    private:
        size_t m_width;
        size_t m_height;
        size_t m_depth;
        T* m_begin;
    };

    /// <summary>
    /// Allows to access some contiguous boolean data as a 3D array with bit-compression.
    /// </summary>
    /// <typeparam name="UnderlyingDataType">Underlying data type</typeparam>
    template<typename UnderlyingDataType>
    class Array3DViewBool {
    public:
        Array3DViewBool(size_t width, size_t height, size_t depth, UnderlyingDataType* begin, size_t bit_offset)
            :m_width(width), m_height(height), m_depth(depth), m_begin(begin), m_bit_offset(bit_offset) {}

        Array3DViewBool(glm::ivec3 dims, UnderlyingDataType* begin, size_t bit_offset)
            :Array3DViewBool(dims.x, dims.y, dims.z, begin, bit_offset) {}

        bool Get(size_t i, size_t j, size_t k) {
            size_t bit_offset = GetTotalBitOffset(i, j, k);
            return ((*(m_begin + (bit_offset / BIT_SIZE))) >> (bit_offset & (BIT_SIZE - 1))) & 1;
        }

        void Set(size_t i, size_t j, size_t k, bool value) const {
            size_t bit_offset = GetTotalBitOffset(i, j, k);
            if (value) {
                *(m_begin + (bit_offset / BIT_SIZE)) |= (1ull << (bit_offset & (BIT_SIZE - 1)));
            }
            else {
                *(m_begin + (bit_offset / BIT_SIZE)) &= ~(1ull << (bit_offset & (BIT_SIZE - 1)));
            }
        }

        void Fill(bool value) {
            for (int i = 0; i < m_width; i++) {
                for (int j = 0; j < m_width; j++) {
                    for (int k = 0; k < m_width; k++) {
                        Set(i, j, k, value);
                    }
                }
            }
        }

    private:

        static constexpr size_t BIT_SIZE = sizeof(UnderlyingDataType) * 8;

        size_t GetTotalBitOffset(size_t i, size_t j, size_t k) const {
            return m_bit_offset + i * m_height * m_depth + j * m_depth + k;
        }

        size_t m_width;
        size_t m_height;
        size_t m_depth;
        size_t m_bit_offset;
        UnderlyingDataType* m_begin;
    };

}