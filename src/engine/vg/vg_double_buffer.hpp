#pragma once

#include "vg_buffer.hpp"

namespace Engine {
class VgDoubleBuffer {
public:
    using CreateInfo = VgBuffer::CreateInfo;

    VgDoubleBuffer() = default;
    explicit VgDoubleBuffer(const Config& config, VgDevice& device, const CreateInfo& createInfo);
    ~VgDoubleBuffer();
    VgDoubleBuffer(const VgDoubleBuffer& other) = delete;
    VgDoubleBuffer(VgDoubleBuffer&& other) noexcept;
    VgDoubleBuffer& operator=(const VgDoubleBuffer& other) = delete;
    VgDoubleBuffer& operator=(VgDoubleBuffer&& other) noexcept;
    void swap(VgDoubleBuffer& other) noexcept;
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
    std::array<VgBuffer, MAX_FRAMES_IN_FLIGHT> buffers;
};
} // namespace Engine
