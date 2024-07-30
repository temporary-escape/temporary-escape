#pragma once

#include "VulkanBuffer.hpp"

namespace Engine {
class ENGINE_API VulkanRenderer;

class ENGINE_API VulkanDoubleBuffer : public VulkanDisposable {
public:
    using Buffers = std::array<VulkanBuffer, MAX_FRAMES_IN_FLIGHT>;
    using CreateInfo = VulkanBuffer::CreateInfo;

    VulkanDoubleBuffer() = default;
    explicit VulkanDoubleBuffer(VulkanRenderer& device, const CreateInfo& createInfo);
    ~VulkanDoubleBuffer();
    VulkanDoubleBuffer(const VulkanDoubleBuffer& other) = delete;
    VulkanDoubleBuffer(VulkanDoubleBuffer&& other) noexcept;
    VulkanDoubleBuffer& operator=(const VulkanDoubleBuffer& other) = delete;
    VulkanDoubleBuffer& operator=(VulkanDoubleBuffer&& other) noexcept;
    void swap(VulkanDoubleBuffer& other) noexcept;
    void destroy() override;
    void subDataLocal(const void* data, size_t offset, size_t size);
    void* mapMemory();
    void unmapMemory();
    VulkanBuffer& getCurrentBuffer();
    const VulkanBuffer& getCurrentBuffer() const;
    VulkanBuffer& getPreviousBuffer();
    const VulkanBuffer& getPreviousBuffer() const;

    const Buffers& getBuffers() const {
        return buffers;
    }

    VkBuffer& getHandle() {
        return getCurrentBuffer().getHandle();
    }

    const VkBuffer& getHandle() const {
        return getCurrentBuffer().getHandle();
    }

    size_t getSize() const {
        if (!device) {
            return 0;
        }
        return getCurrentBuffer().getSize();
    }

    operator bool() const {
        return device != nullptr;
    }

private:
    VulkanRenderer* device{nullptr};
    Buffers buffers;
};
} // namespace Engine
