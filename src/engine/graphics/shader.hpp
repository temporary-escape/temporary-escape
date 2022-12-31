#pragma once

#include "../vulkan/vulkan_renderer.hpp"

namespace Engine {
class Shader {
public:
    Shader() = default;

    operator bool() const {
        return !!pipeline && !!descriptorSetLayout;
    }

protected:
    VulkanPipeline pipeline;
    VulkanDescriptorSetLayout descriptorSetLayout;
};
} // namespace Engine
