#include <lit/engine/components/voxel_grid/voxel_grid_base.hpp>
#include <functional>
#include <glm/glm.hpp>

namespace lit::engine {

    const std::function<uint32_t(uint32_t, uint32_t)> MERGE_KEEP_SECOND = [](uint32_t a, uint32_t b) {return b;};
    const std::function<uint32_t(uint32_t, uint32_t)> MERGE_AVERAGE_PER_BYTE = [](uint32_t a, uint32_t b) {
        return (((((a >> 0) & 0xFF) + ((b >> 0) & 0xFF)) / 2) << 0) +
                (((((a >> 8) & 0xFF) + ((b >> 8) & 0xFF)) / 2) << 8) +
                (((((a >> 16) & 0xFF) + ((b >> 16) & 0xFF)) / 2) << 16) +
                (((((a >> 24) & 0xFF) + ((b >> 24) & 0xFF)) / 2) << 24);
    };

    template<typename VoxelType>
    void Merge(VoxelGridBaseT <VoxelType> &a, const VoxelGridBaseT <VoxelType> &b,
               std::function<VoxelType(VoxelType, VoxelType)> mergeFunc,
               glm::dvec3 offset) {
        glm::ivec3 offsetInt = glm::floor(offset - b.GetAnchor() + a.GetAnchor() + 0.5);

        auto aDim = a.GetDimensions();
        auto bDim = b.GetDimensions();

        for (int x = std::min(0, -offsetInt.x); x < std::max(bDim.x, aDim.x - offsetInt.x); x++) {
            for (int y = std::min(0, -offsetInt.y); y < std::max(bDim.y, aDim.y - offsetInt.y); y++) {
                for (int z = std::min(0, -offsetInt.z); z < std::max(bDim.z, aDim.z - offsetInt.z); z++) {
                    VoxelType aVal = a.GetVoxel(glm::ivec3(x, y, z) + offsetInt);
                    VoxelType bVal = b.GetVoxel({x, y, z});
                    if (bVal && aVal) {
                        a.SetVoxel(glm::ivec3(x, y, z) + offsetInt, mergeFunc(aVal, bVal));
                    } else if (bVal) {
                        a.SetVoxel(glm::ivec3(x, y, z) + offsetInt, bVal);
                    }
                }
            }
        }
    }
}

