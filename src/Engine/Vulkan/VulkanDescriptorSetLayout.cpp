#include "VulkanDescriptorSetLayout.hpp"
#include "../Utils/Exceptions.hpp"
#include "VulkanDevice.hpp"

using namespace Engine;

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDevice& device, const CreateInfo& createInfo) :
    device{device.getDevice()} {

    if (vkCreateDescriptorSetLayout(device.getDevice(), &createInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        EXCEPTION("Failed to create descriptor set layout!");
    }
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
    destroy();
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept {
    swap(other);
}

VulkanDescriptorSetLayout& VulkanDescriptorSetLayout::operator=(VulkanDescriptorSetLayout&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanDescriptorSetLayout::swap(VulkanDescriptorSetLayout& other) noexcept {
    std::swap(device, other.device);
    std::swap(descriptorSetLayout, other.descriptorSetLayout);
}

void VulkanDescriptorSetLayout::destroy() {
    if (descriptorSetLayout) {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        descriptorSetLayout = VK_NULL_HANDLE;
    }
}
