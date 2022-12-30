#pragma once

#include "../utils/path.hpp"
#include "vg_types.hpp"

namespace Engine {
class VgRenderer;

class VgAllocator {
public:
    VgAllocator() = default;
    explicit VgAllocator(const Config& config, VgRenderer& device);
    ~VgAllocator();
    VgAllocator(const VgAllocator& other) = delete;
    VgAllocator(VgAllocator&& other) noexcept;
    VgAllocator& operator=(const VgAllocator& other) = delete;
    VgAllocator& operator=(VgAllocator&& other) noexcept;
    void swap(VgAllocator& other) noexcept;

    VmaAllocator& getHandle() {
        return allocator;
    }

    void destroy();

private:
    VmaAllocator allocator{VK_NULL_HANDLE};
};
} // namespace Engine
