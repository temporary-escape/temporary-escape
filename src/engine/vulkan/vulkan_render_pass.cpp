#include "vulkan_render_pass.hpp"
#include "../utils/exceptions.hpp"
#include "vulkan_device.hpp"

using namespace Engine;

VulkanRenderPass::VulkanRenderPass(VulkanDevice& device, const CreateInfo& createInfo) : device{device.getDevice()} {
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(createInfo.attachments.size());
    renderPassInfo.pAttachments = createInfo.attachments.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(createInfo.subPasses.size());
    renderPassInfo.pSubpasses = createInfo.subPasses.data();
    renderPassInfo.dependencyCount = static_cast<uint32_t>(createInfo.dependencies.size());
    renderPassInfo.pDependencies = createInfo.dependencies.data();

    if (vkCreateRenderPass(device.getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        EXCEPTION("Failed to create render pass!");
    }
}

VulkanRenderPass::~VulkanRenderPass() {
    destroy();
}

VulkanRenderPass::VulkanRenderPass(VulkanRenderPass&& other) noexcept {
    swap(other);
}

VulkanRenderPass& VulkanRenderPass::operator=(VulkanRenderPass&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanRenderPass::swap(VulkanRenderPass& other) noexcept {
    std::swap(device, other.device);
    std::swap(renderPass, other.renderPass);
}

void VulkanRenderPass::destroy() {
    if (renderPass) {
        vkDestroyRenderPass(device, renderPass, nullptr);
        renderPass = VK_NULL_HANDLE;
    }
}
