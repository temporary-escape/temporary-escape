#pragma once

#include "vulkan_double_buffer.hpp"

namespace Engine {

class ENGINE_API VulkanArrayBuffer : public VulkanDisposable {
public:
    struct CreateInfo {
        size_t stride{0};
    };

    VulkanArrayBuffer() = default;
    explicit VulkanArrayBuffer(size_t stride);
    ~VulkanArrayBuffer() override;
    NON_COPYABLE(VulkanArrayBuffer);
    MOVEABLE(VulkanArrayBuffer);
    void destroy() override;

    void recalculate(VulkanRenderer& vulkan);
    void* insert(const uint64_t id);
    void remove(const uint64_t id);
    size_t count() const;
    VulkanBuffer& getCurrentBuffer() {
        return vbo.getCurrentBuffer();
    }
    const VulkanBuffer& getCurrentBuffer() const {
        return vbo.getCurrentBuffer();
    }

private:
    VulkanRenderer* vulkan{nullptr};
    size_t stride{0};
    VulkanDoubleBuffer vbo;
    std::unordered_map<uint64_t, uint64_t> indexMap;
    std::vector<uint8_t> buffer;
    size_t flush{false};
    size_t end{0};
};
} // namespace Engine
