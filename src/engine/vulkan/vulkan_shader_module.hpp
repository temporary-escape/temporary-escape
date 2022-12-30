#pragma once

#include "../utils/path.hpp"
#include "vulkan_types.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanShaderModule : public VulkanDisposable {
public:
    VulkanShaderModule() = default;
    explicit VulkanShaderModule(const Config& config, VulkanDevice& device, const std::string& glsl,
                                VkShaderStageFlagBits stage);
    explicit VulkanShaderModule(const Config& config, VulkanDevice& device, const Path& path,
                                VkShaderStageFlagBits stage);
    ~VulkanShaderModule();
    VulkanShaderModule(const VulkanShaderModule& other) = delete;
    VulkanShaderModule(VulkanShaderModule&& other) noexcept;
    VulkanShaderModule& operator=(const VulkanShaderModule& other) = delete;
    VulkanShaderModule& operator=(VulkanShaderModule&& other) noexcept;
    void swap(VulkanShaderModule& other) noexcept;

    VkShaderStageFlagBits getStage() const {
        return stage;
    }

    VkShaderModule& getHandle() {
        return shaderModule;
    }

    const VkShaderModule& getHandle() const {
        return shaderModule;
    }

    operator bool() const {
        return shaderModule != VK_NULL_HANDLE;
    }

    void destroy();

private:
    VkDevice device{VK_NULL_HANDLE};
    VkShaderModule shaderModule{VK_NULL_HANDLE};
    VkShaderStageFlagBits stage;
};
} // namespace Engine
