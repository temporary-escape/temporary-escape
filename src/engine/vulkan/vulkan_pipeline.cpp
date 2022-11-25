#include "vulkan_pipeline.hpp"
#include "../utils/exceptions.hpp"

using namespace Engine;

VulkanPipeline::VulkanPipeline(VkDevice device, VezPipeline pipeline, std::vector<VkShaderModule> shaderModules)
    : device(device), desc{pipeline, std::move(shaderModules)} {
}

VulkanPipeline::~VulkanPipeline() {
    reset();
}

VulkanPipeline::VulkanPipeline(VulkanPipeline&& other) noexcept {
    swap(other);
}

VulkanPipeline& VulkanPipeline::operator=(VulkanPipeline&& other) noexcept {
    if (this != &other) {
        swap(other);
    }

    return *this;
}

void VulkanPipeline::swap(VulkanPipeline& other) noexcept {
    std::swap(desc, other.desc);
    std::swap(device, other.device);
}

void VulkanPipeline::reset() {
    if (device && desc.pipeline) {
        vezDestroyPipeline(device, desc.pipeline);
        for (auto shaderModule : desc.shaderModules) {
            vezDestroyShaderModule(device, shaderModule);
        }
    }

    device = VK_NULL_HANDLE;
    desc.pipeline = VK_NULL_HANDLE;
    desc.shaderModules.clear();
}
