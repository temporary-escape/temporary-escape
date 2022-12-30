#include "vg_render_pass.hpp"
#include "../utils/exceptions.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgRenderPass::VgRenderPass(const Config& config, VgDevice& device, const CreateInfo& createInfo) : device{&device} {
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(createInfo.attachments.size());
    renderPassInfo.pAttachments = createInfo.attachments.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(createInfo.subPasses.size());
    renderPassInfo.pSubpasses = createInfo.subPasses.data();

    if (vkCreateRenderPass(device.getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        EXCEPTION("Failed to create render pass!");
    }
}

VgRenderPass::~VgRenderPass() {
    destroy();
}

VgRenderPass::VgRenderPass(VgRenderPass&& other) noexcept {
    swap(other);
}

VgRenderPass& VgRenderPass::operator=(VgRenderPass&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgRenderPass::swap(VgRenderPass& other) noexcept {
    std::swap(device, other.device);
    std::swap(renderPass, other.renderPass);
}

void VgRenderPass::destroy() {
    if (renderPass) {
        vkDestroyRenderPass(device->getDevice(), renderPass, nullptr);
        renderPass = VK_NULL_HANDLE;
    }
}
