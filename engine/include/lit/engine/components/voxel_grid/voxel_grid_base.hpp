#pragma once

#include <functional>
#include <lit/common/images/images.hpp>

namespace lit::engine {

    /**
     * @VoxelGridBaseT is an abstract class that represents some voxel space with fixed dimensions.
     * Each voxel in grid can store a single value of type @VoxelType.
     *
     * Anchor:
     *  Each voxel grid has an anchor that is defined by some point inside (or outside) of the grid.
     *  Anchor is the "relative" center of this voxel space, it is the center of translations and rotations
     *  in case if this grid will be placed in some 3D space.
     *
     * Dimensions:
     *  Fixed numbers of voxels along each axis that this grid can represent:
     *  x - width
     *  y - height
     *  z - depth
     *
     *            *---------*
     *           /|        /|
     *          / |       / |
     *         /  |      /  |
     *        *---------*   |
     *        |   *-----|---*
     * height |  /      |  /
     *        | /       | / depth
     *        |/        |/
     *        *---------*
     *          width
     */
    template<typename VoxelType>
    class VoxelGridBaseT {
    public:

        VoxelGridBaseT(const glm::ivec3 &dimensions, const glm::dvec3 &anchor)
                : m_dimensions(dimensions), m_anchor(anchor) {}

        virtual ~VoxelGridBaseT() = default;

        /**
         * Set voxel at position @pos to @value
         */
        virtual void SetVoxel(const glm::ivec3 &pos, VoxelType value) = 0;

        /**
         * Get voxel value at position @pos
         */
        virtual VoxelType GetVoxel(const glm::ivec3 &pos) const = 0;

        /**
         * Get grid dimensions
         */
        glm::ivec3 GetDimensions() const {
            return m_dimensions;
        }

        /**
         * Get anchor of the grid relatively to position (0, 0, 0) in local coordinates.
         * Anchor is the center of rotation for voxel grid.
         */
        glm::dvec3 GetAnchor() const {
            return m_anchor;
        }

        // TODO: delete?
        using VoxelChangedCallback = std::function<void(const glm::ivec3 &, VoxelType)>;

        // TODO: delete?
        size_t AddOnVoxelChangedCallback(VoxelChangedCallback callback) {
            m_voxel_changed_callbacks.emplace_back(std::move(callback));
            return m_voxel_changed_callbacks.size() - 1;
        }

        // TODO: delete?
        void RemoveOnVoxelChangedCallback(size_t index) {
            VoxelChangedCallback().swap(m_voxel_changed_callbacks[index]);
        }

        /**
         * Get size of the structure in bytes.
         */
        virtual size_t GetSizeBytes() const {
            return sizeof(VoxelGridBaseT<VoxelType>) +
                   m_voxel_changed_callbacks.capacity() * sizeof(VoxelChangedCallback);
        }

    protected:

        // TODO: delete?
        void InvokeOnVoxelChangedCallbacks(const glm::ivec3 &pos, VoxelType value) {
            for (auto &callback: m_voxel_changed_callbacks) {
                if (callback) {
                    callback(pos, value);
                }
            }
        }

        glm::ivec3 m_dimensions = {1, 1, 1};
        glm::dvec3 m_anchor = {0.0, 0.0, 0.0};

    private:

        // TODO: delete?
        std::vector<VoxelChangedCallback> m_voxel_changed_callbacks;
    };

}