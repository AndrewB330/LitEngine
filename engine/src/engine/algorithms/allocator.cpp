#include <lit/engine/algorithms/allocator.hpp>
#include <spdlog/spdlog.h>

using namespace lit::engine;

const int step = 16;

Allocator3D::Allocator3D(glm::ivec3 size) : m_storage(size / step), m_size(size) {

}

glm::ivec3 Allocator3D::Allocate(glm::ivec3 size) {
    if (glm::any(glm::greaterThan(size, m_size))) {
        return INVALID_TEXTURE_ADDRESS;
    }

    for (int i = 0; i + size.x <= m_size.x; i += step) {
        for (int j = 0; j + size.y <= m_size.y; j += step) {
            for (int k = 0; k + size.z <= m_size.z; k += step) {
                glm::ivec3 pos = {i, j, k};
                auto res = m_storage.Get(iregion3{pos / step, (pos + size + glm::ivec3(step - 1))/step});
                if (res == 0) {
                    m_storage.Add(iregion3{pos/step, (pos + size + glm::ivec3(step - 1))/step}, 1);
                    return pos;
                }
            }
        }
    }

    return INVALID_TEXTURE_ADDRESS;
}

void Allocator3D::Free(glm::ivec3 address, glm::ivec3 size) {
    m_storage.Add(iregion3{address/step, (address + size + glm::ivec3(step - 1))/step}, -1);
}

FixedAllocator3D::FixedAllocator3D(glm::ivec3 size, glm::ivec3 cell)
        : m_grid(size / cell), m_cell(cell), m_address(0), m_max_address(glm::compMul(m_grid)) {}

glm::ivec3 FixedAllocator3D::Allocate() {
    if (!m_address_pool.empty()) {
        auto res = m_address_pool.back();
        m_address_pool.pop_back();
        return Convert(res);
    }

    if (m_address == m_max_address) {
        spdlog::default_logger()->error("FixedAllocator3D overflow");
        return INVALID_TEXTURE_ADDRESS;
    }
    return Convert(m_address++);
}

void FixedAllocator3D::Free(glm::ivec3 address) {
    m_address_pool.push_back(Convert(address));
}

uint32_t FixedAllocator3D::Convert(glm::ivec3 address) {
    auto g = address / m_cell;
    return g.x + g.y * m_grid.x + g.z * m_grid.x * m_grid.y;
}

glm::ivec3 FixedAllocator3D::Convert(uint32_t address) {
    uint32_t x = address % m_grid.x;
    address /= m_grid.x;
    uint32_t y = address % m_grid.y;
    address /= m_grid.y;
    return m_cell * glm::ivec3(x, y, address);
}

FixedAllocator::FixedAllocator(uint32_t n):m_size(n == 0 ? std::numeric_limits<uint32_t>::max() : n) {}

uint32_t FixedAllocator::Allocate() {
    if (m_pool.empty()) {
        if (m_ptr >= m_size) {
            spdlog::default_logger()->error("Allocator out of bounds");
        }
        return m_ptr++;
    }
    uint32_t res = m_pool.back();
    m_pool.pop_back();
    return res;
}

void FixedAllocator::Free(uint32_t address) {
    m_pool.push_back(address);
}

size_t FixedAllocator::GetSizeBytes() const {
    return sizeof(FixedAllocator) + m_pool.capacity() * sizeof(uint32_t);
}

bool FixedAllocator::CanAllocate() {
    return m_ptr < m_size || !m_pool.empty();
}

uint32_t FixedAllocator::GetSize() const {
    return m_size;
}

uint32_t FixedAllocator::GetPtr() const {
    return m_ptr;
}
