#pragma once

#include <lit/engine/components/voxel_grid/voxel_grid_base.hpp>
#include <lit/engine/utilities/array_view.hpp>
#include <glm/gtx/component_wise.hpp>
#include "glm/vector_relational.hpp"

namespace lit::engine {

    template<typename VoxelType>
    class VoxelGridDenseT : public VoxelGridBaseT<uint32_t> {
    public:
        VoxelGridDenseT(const glm::ivec3 &dimensions, const glm::dvec3 &anchor)
                : VoxelGridBaseT<VoxelType>(dimensions, anchor),
                  m_data(glm::compMul(VoxelGridBaseT<VoxelType>::m_dimensions), 0),
                  m_view(VoxelGridBaseT<VoxelType>::m_dimensions, &m_data[0], &m_data[0] + m_data.size()) {
        }

        ~VoxelGridDenseT() override = default;

        void SetVoxel(const glm::ivec3 &pos, VoxelType value) override {
            if (glm::any(glm::lessThan(pos, glm::ivec3(0))) || glm::any(glm::greaterThanEqual(pos, m_dimensions))) {
                return;
            }

            m_view.At(pos) = value;
        };

        VoxelType GetVoxel(const glm::ivec3 &pos) const override {
            if (glm::any(glm::lessThan(pos, glm::ivec3(0))) || glm::any(glm::greaterThanEqual(pos, m_dimensions))) {
                return 0;
            }

            return m_view.At(pos);
        }

        size_t GetSizeBytes() const override {
            return VoxelGridBaseT<VoxelType>::GetSizeBytes()
                   - sizeof(VoxelGridBaseT<VoxelType>)
                   + sizeof(VoxelGridDenseT<VoxelType>)
                   + m_data.capacity() * sizeof(VoxelType)
                   + sizeof(std::vector<VoxelType>)
                   + sizeof(Array3DView<VoxelType>);
        }

    private:
        std::vector<VoxelType> m_data;
        Array3DView<VoxelType> m_view;
    };

}