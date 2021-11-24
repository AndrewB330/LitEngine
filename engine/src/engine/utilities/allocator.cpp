#include <lit/engine/utilities/allocator.hpp>
#include <spdlog/spdlog.h>

using namespace lit::engine;

ContiguousAllocator::ContiguousAllocator(uint32_t n):m_size(n == 0 ? std::numeric_limits<uint32_t>::max() : n) {}

uint32_t ContiguousAllocator::Allocate() {
    if (m_pool.empty()) {
        if (m_ptr >= m_size) {
            spdlog::default_logger()->error("Allocator out of bounds, size: {}", m_size);
        }
        return m_ptr++;
    }
    uint32_t res = m_pool.back();
    m_pool.pop_back();
    return res;
}

void ContiguousAllocator::Free(uint32_t address) {
    m_pool.push_back(address);
}

size_t ContiguousAllocator::GetSizeBytes() const {
    return sizeof(ContiguousAllocator) + m_pool.capacity() * sizeof(uint32_t);
}

bool ContiguousAllocator::CanAllocate() {
    return m_ptr < m_size || !m_pool.empty();
}

uint32_t ContiguousAllocator::GetSize() const {
    return m_size;
}

uint32_t ContiguousAllocator::GetPtr() const {
    return m_ptr;
}
