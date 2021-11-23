#pragma once

#include <functional>
#include <lit/common/images/images.hpp>

namespace lit::engine {

    template<typename VoxelType>
    class RawVoxelGridViewT {
    public:
        RawVoxelGridViewT(glm::ivec3 dimensions, VoxelType* begin) :m_dimensions(dimensions), m_begin(begin) {}

        VoxelType& At(glm::ivec3 position) {
            // todo: potential bitshift optimizaion?
            return *(m_begin + (position.x * m_dimensions.y * m_dimensions.z +
                position.y * m_dimensions.z +
                position.z));
        }

        const VoxelType& At(glm::ivec3 position) const {
            // todo: potential bitshift optimizaion?
            return *(m_begin + (position.x * m_dimensions.y * m_dimensions.z +
                position.y * m_dimensions.z +
                position.z));
        }

    private:
        VoxelType* m_begin;
        glm::ivec3 m_dimensions;
    };

    template<typename UnderlyingDataType>
    class RawCompressedVoxelGridViewT {
    public:
        RawCompressedVoxelGridViewT(glm::ivec3 dimensions, UnderlyingDataType* begin, size_t bit_offset)
            :m_dimensions(dimensions), m_begin(begin), m_bit_offset(bit_offset) {}

        bool Get(glm::ivec3 position) {
            size_t bit_offset = GetTotalBitOffset(position);
            return m_begin + (bit_offset >> (sizeof(UnderlyingDataType) * 8));
        }

        void Set(glm::ivec3 position, bool value) const {
            size_t bit_offset = GetTotalBitOffset(position);
        }

    private:

        size_t GetTotalBitOffset(glm::ivec3 position) {
            // todo: potential bitshift optimizaion?
            return m_bit_offset + (position.x * m_dimensions.y * m_dimensions.z +
                position.y * m_dimensions.z +
                position.z);
        }

        UnderlyingDataType* m_begin;
        glm::ivec3 m_dimensions;
        size_t m_bit_offset;
    };

    template<typename VoxelType>
    class VoxelGridBaseT {
    public:

        VoxelGridBaseT(const glm::ivec3& dimensions, const glm::dvec3& anchor)
            :m_dimensions(dimensions), m_anchor(anchor) {}

        virtual void SetVoxel(const glm::ivec3& pos, VoxelType value) = 0;

        virtual VoxelType GetVoxel(const glm::ivec3& pos) const = 0;

        glm::ivec3 GetDimensions() const {
            return m_dimensions;
        }

        glm::dvec3 GetAnchor() const {
            return m_anchor;
        }

        using VoxelChangedCallback = std::function<void(const glm::ivec3&, VoxelType)>;

        size_t AddOnVoxelChangedCallback(VoxelChangedCallback callback) {
            m_voxel_changed_callbacks.emplace_back(std::move(callback));
            return m_voxel_changed_callbacks.size() - 1;
        }

        void RemoveOnVoxelChangedCallback(size_t index) {
            VoxelChangedCallback().swap(m_voxel_changed_callbacks[index]);
        }

        virtual size_t GetSizeBytes() const {
            return sizeof(VoxelGridBaseT<VoxelType>) +
                m_voxel_changed_callbacks.capacity() * sizeof(VoxelChangedCallback);
        }

    protected:

        void InvokeOnVoxelChangedCallbacks(const glm::ivec3& pos, VoxelType value) {
            for (auto& callback : m_voxel_changed_callbacks) {
                if (callback) {
                    callback(pos, value);
                }
            }
        }

    private:

        glm::ivec3 m_dimensions = { 1, 1, 1 };
        glm::dvec3 m_anchor = { 0.0, 0.0, 0.0 };

        std::vector<VoxelChangedCallback> m_voxel_changed_callbacks;
    };

}