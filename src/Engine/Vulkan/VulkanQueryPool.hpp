#pragma once

#include "VulkanBuffer.hpp"

namespace Engine {
class ENGINE_API VulkanRenderer;

class ENGINE_API VulkanQueryPool : public VulkanDisposable {
public:
    using CreateInfo = VkQueryPoolCreateInfo;

    VulkanQueryPool() = default;
    explicit VulkanQueryPool(VulkanRenderer& device, const CreateInfo& createInfo);
    ~VulkanQueryPool();
    VulkanQueryPool(const VulkanQueryPool& other) = delete;
    VulkanQueryPool(VulkanQueryPool&& other) noexcept;
    VulkanQueryPool& operator=(const VulkanQueryPool& other) = delete;
    VulkanQueryPool& operator=(VulkanQueryPool&& other) noexcept;
    void swap(VulkanQueryPool& other) noexcept;
    void destroy() override;

    VkQueryPool& getHandle();
    const VkQueryPool& getHandle() const;

    template <typename T>
    std::vector<uint64_t> getResult(uint32_t firstQuery, uint32_t count, VkQueryResultFlags flags);

private:
    VulkanRenderer* device{nullptr};
    std::array<VkQueryPool, MAX_FRAMES_IN_FLIGHT> pools;
};
} // namespace Engine
