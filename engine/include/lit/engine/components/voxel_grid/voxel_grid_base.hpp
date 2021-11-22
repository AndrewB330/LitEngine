#pragma once

#include <functional>
#include <lit/common/images/images.hpp>

namespace lit::engine {

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