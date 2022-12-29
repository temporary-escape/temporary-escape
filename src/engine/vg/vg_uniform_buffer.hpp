#pragma once

#include "vg_buffer.hpp"

namespace Engine {
class VgUniformBuffer {
public:
    enum class Usage {
        Static,
        Dynamic,
    };

    VgUniformBuffer() = default;
    explicit VgUniformBuffer(const Config& config, VgDevice& device, size_t size, Usage usage);
    ~VgUniformBuffer();
    VgUniformBuffer(const VgUniformBuffer& other) = delete;
    VgUniformBuffer(VgUniformBuffer&& other) noexcept;
    VgUniformBuffer& operator=(const VgUniformBuffer& other) = delete;
    VgUniformBuffer& operator=(VgUniformBuffer&& other) noexcept;
    void swap(VgUniformBuffer& other) noexcept;
    void destroy();
    void subData(const void* data, size_t offset, size_t size);
    VgBuffer& getCurrentBuffer();
    const VgBuffer& getCurrentBuffer() const;

    VkBuffer& getHandle() {
        return getCurrentBuffer().getHandle();
    }

    const VkBuffer& getHandle() const {
        return getCurrentBuffer().getHandle();
    }

    size_t getSize() const {
        return getCurrentBuffer().getSize();
    }

    operator bool() const {
        return !buffers.empty();
    }

private:
    VgDevice* device{nullptr};
    std::vector<VgBuffer> buffers;
};

struct VgUniformBufferBinding {
    uint32_t binding{0};
    VgUniformBuffer* uniform{nullptr};
};
} // namespace Engine
