#pragma once

#include <vector>
#include <lit/common/glm_ext/region.hpp>

namespace lit::common::algorithms {

    using iregion3 = lit::common::glm_ext::iregion3;

    class FenwickTreeRange3D {
    public:
        explicit FenwickTreeRange3D(glm::ivec3 dims);

        void Reset(glm::ivec3 dims);

        void Add(iregion3 region, int value);

        void Add(glm::ivec3 pos, int value);

        int Get(iregion3 region);

    private:
        int Get(glm::ivec3 end);

        int PosToIndex(int i, int j, int k) const;

        std::vector<int> data_m;
        glm::ivec3 dims_m;
    };

}
