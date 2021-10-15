#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <lit/common/glm_ext/region.hpp>
#include <lit/common/images/image.hpp>

namespace lit::voxels {

    using lit::common::glm_ext::iregion3;

    template<typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
    class VoxelGridT {
    public:
        static const T kDefaultValue = T(0);

        explicit VoxelGridT(glm::ivec3 dimensions, T fillValue = kDefaultValue)
                : m_data(dimensions.x, dimensions.y, dimensions.z, fillValue), m_dimensions(dimensions) {}

        glm::ivec3 GetDimensions() const {
            return m_dimensions;
        }

        T GetVoxel(int x, int y, int z) const {
            return m_data.get_pixel(x, y, z);
        }

        T GetVoxel(glm::ivec3 pos) const {
            return m_data.get_pixel(pos);
        }

        void SetVoxel(int x, int y, int z, T value) {
            m_data.set_pixel(x, y, z, value);
        }

        void SetVoxel(glm::ivec3 pos, T value) {
            m_data.set_pixel(pos, value);
        }

    private:
        lit::common::image3d<T> m_data;
        glm::ivec3 m_dimensions;
    };

    template<>
    class VoxelGridT<bool> {
    public:
        static const bool kDefaultValue = false;

        explicit VoxelGridT(glm::ivec3 dimensions, bool fillValue = kDefaultValue)
                : m_compressed_data(std::max(1, dimensions.x >> 1), std::max(dimensions.y >> 1, 1), std::max(dimensions.z >> 1, 1), fillValue * 0xFF), m_dimensions(dimensions) {}

        glm::ivec3 GetDimensions() const {
            return m_dimensions;
        }

        bool GetVoxel(int x, int y, int z) const {
            return (m_compressed_data.get_pixel(x >> 1, y >> 1, z >> 1) >> GetDataBit(x, y, z)) & 1;
        }

        bool GetVoxel(glm::ivec3 pos) const {
            return (m_compressed_data.get_pixel(pos >> 1) >> GetDataBit(pos.x, pos.y, pos.z)) & 1;
        }

        uint8_t GetVoxelBlock(int x, int y, int z) const {
            return m_compressed_data.get_pixel(x >> 1, y >> 1, z >> 1);
        }

        void SetVoxel(int x, int y, int z, bool value) {
            int bit = GetLodDataBit(x, y, z);
            m_compressed_data.get_pixel(x >> 1, y >> 1, z >> 1) &= ~(1u << bit);
            m_compressed_data.get_pixel(x >> 1, y >> 1, z >> 1) |= (value << bit);
        }

        void SetVoxel(glm::ivec3 pos, bool value) {
            int bit = GetLodDataBit(pos.x, pos.y, pos.z);
            m_compressed_data.get_pixel(pos >> 1) &= ~(1u << bit);
            m_compressed_data.get_pixel(pos >> 1) |= (value << bit);
        }

    private:

        inline int VoxelGrid::GetDataBit(int x, int y, int z) const {
            return (x & 1) | ((y & 1) << 1) | ((z & 1) << 2);
        }

        lit::common::image3d<uint8_t> m_compressed_data;
        glm::ivec3 m_dimensions;
    };

    typedef VoxelGridT<uint8_t> VoxelGridByte;
    typedef VoxelGridT<bool> VoxelGridBool;

    using template VoxelGridT<bool>;
    using template VoxelGridT<uint8_t>;
    using template VoxelGridT<uint32_t>;

}