#include <lit/engine/algorithms/fenwick_tree.hpp>

using namespace lit::engine;

FenwickTreeRange3D::FenwickTreeRange3D(glm::ivec3 dims): dims_m(dims) {
    Reset(dims);
}

void FenwickTreeRange3D::Reset(glm::ivec3 dims) {
    dims_m = dims;
    data_m.assign(glm::compMul(dims), 0);
}

int FenwickTreeRange3D::Get(glm::ivec3 end) {
    int res = 0;
    for (int i = end.x - 1; i >= 0; i = (i & (i + 1)) - 1)
        for (int j = end.y - 1; j >= 0; j = (j & (j + 1)) - 1)
            for (int k = end.z - 1; k >= 0; k = (k & (k + 1)) - 1)
                res += data_m.at(PosToIndex(i, j, k));
    return res;
}

int FenwickTreeRange3D::Get(iregion3 region) {
    return Get(region.end)
           - Get(glm::ivec3(region.begin.x, region.end.y, region.end.z))
           - Get(glm::ivec3(region.end.x, region.begin.y, region.end.z))
           - Get(glm::ivec3(region.end.x, region.end.y, region.begin.z))
           + Get(glm::ivec3(region.end.x, region.begin.y, region.begin.z))
           + Get(glm::ivec3(region.begin.x, region.end.y, region.begin.z))
           + Get(glm::ivec3(region.begin.x, region.begin.y, region.end.z))
           - Get(glm::ivec3(region.begin.x, region.begin.y, region.begin.z));
}

void FenwickTreeRange3D::Add(iregion3 region, int value) {
    for (int64_t i = region.begin.x; i < region.end.x; i++)
        for (int64_t j = region.begin.y; j < region.end.y; j++)
            for (int64_t k = region.begin.z; k < region.end.z; k++)
                Add(glm::ivec3(i, j, k), value);
}

void FenwickTreeRange3D::Add(glm::ivec3 pos, int value) {
    for (int64_t i = pos.x; i < dims_m.x; i = (i | (i + 1)))
        for (int64_t j = pos.y; j < dims_m.y; j = (j | (j + 1)))
            for (int64_t k = pos.z; k < dims_m.z; k = (k | (k + 1)))
                data_m.at(PosToIndex(i, j, k)) += value;
}

int64_t FenwickTreeRange3D::PosToIndex(int64_t i, int64_t j, int64_t k) const {
    return i + j * dims_m.x + k * dims_m.x * dims_m.y;
}
