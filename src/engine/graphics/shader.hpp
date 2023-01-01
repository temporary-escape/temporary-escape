#pragma once

#include "../vulkan/vulkan_renderer.hpp"

namespace Engine {
class Shader {
public:
    Shader() = default;
    virtual ~Shader() = default;
    Shader(const Shader& other) = delete;
    Shader(Shader&& other) = default;
    Shader& operator=(const Shader& other) = delete;
    Shader& operator=(Shader&& other) = default;

    virtual void finalize(VulkanRenderPass& renderPass) = 0;

    operator bool() const {
        return !!pipeline && !!descriptorSetLayout;
    }

    VulkanPipeline& getPipeline() {
        return pipeline;
    }

    const VulkanPipeline& getPipeline() const {
        return pipeline;
    }

    VulkanDescriptorSetLayout& getDescriptorSetLayout() {
        return descriptorSetLayout;
    }

    const VulkanDescriptorSetLayout& getDescriptorSetLayout() const {
        return descriptorSetLayout;
    }

protected:
    VulkanDescriptorSetLayout descriptorSetLayout;
    VulkanPipeline pipeline;
};
} // namespace Engine
