#pragma once

#include "../Utils/Path.hpp"
#include "SpirvReflection.hpp"
#include "VulkanTypes.hpp"

namespace Engine {
class ENGINE_API VulkanDevice;

class ENGINE_API VulkanShader : public VulkanDisposable {
public:
    VulkanShader() = default;
    explicit VulkanShader(VulkanDevice& device, const Span<uint8_t>& spirv, VkShaderStageFlagBits stage);
    explicit VulkanShader(VulkanDevice& device, const std::string& glsl, VkShaderStageFlagBits stage);
    explicit VulkanShader(VulkanDevice& device, const Path& path, VkShaderStageFlagBits stage);

    ~VulkanShader();
    VulkanShader(const VulkanShader& other) = delete;
    VulkanShader(VulkanShader&& other) noexcept;
    VulkanShader& operator=(const VulkanShader& other) = delete;
    VulkanShader& operator=(VulkanShader&& other) noexcept;
    void swap(VulkanShader& other) noexcept;

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

    const std::vector<uint32_t>& getSpirv() const {
        return spirv;
    }

    void destroy();

    SpirvReflection reflect() const {
        return SpirvReflection{getSpirv()};
    }

private:
    VkDevice device{VK_NULL_HANDLE};
    VkShaderModule shaderModule{VK_NULL_HANDLE};
    VkShaderStageFlagBits stage;
    std::vector<uint32_t> spirv;
};
} // namespace Engine
