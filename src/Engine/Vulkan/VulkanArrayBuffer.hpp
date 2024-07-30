#pragma once

#include "VulkanDoubleBuffer.hpp"

namespace Engine {

class ENGINE_API VulkanArrayBuffer : public VulkanDisposable {
public:
    struct CreateInfo : VulkanBuffer::CreateInfo {
        size_t stride{0};
    };

    VulkanArrayBuffer() = default;
    explicit VulkanArrayBuffer(const CreateInfo& createInfo);
    ~VulkanArrayBuffer() override;
    NON_COPYABLE(VulkanArrayBuffer);
    MOVEABLE(VulkanArrayBuffer);
    void destroy() override;

    bool recalculate(VulkanRenderer& vulkan);
    void* insert(const uint64_t id);
    size_t offsetOf(void* item);
    void remove(const uint64_t id);
    size_t count() const;
    size_t capacity() const;
    bool empty() const;
    VulkanBuffer& getCurrentBuffer() {
        return vbo.getCurrentBuffer();
    }
    const VulkanBuffer& getCurrentBuffer() const {
        return vbo.getCurrentBuffer();
    }
    VulkanDoubleBuffer& getBuffer() {
        return vbo;
    }
    const VulkanDoubleBuffer& getBuffer() const {
        return vbo;
    }
    void clear();
    operator bool() const {
        return createInfo.stride != 0;
    }

private:
    CreateInfo createInfo;
    VulkanDoubleBuffer vbo;
    std::unordered_map<uint64_t, uint64_t> indexMap;
    std::vector<uint8_t> buffer;
    size_t flush{0};
    size_t end{0};
};
} // namespace Engine
