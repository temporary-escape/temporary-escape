#include "vg_descriptor_set_layout.hpp"
#include "../utils/exceptions.hpp"
#include "vg_renderer.hpp"

using namespace Engine;

VgDescriptorSetLayout::VgDescriptorSetLayout(const Config& config, VgRenderer& device, const CreateInfo& createInfo) :
    state{std::make_shared<DescriptionSetLayoutState>()} {

    state->device = &device;

    if (vkCreateDescriptorSetLayout(device.getDevice(), &createInfo, nullptr, &state->descriptorSetLayout) !=
        VK_SUCCESS) {
        EXCEPTION("Failed to create descriptor set layout!");
    }
}

VgDescriptorSetLayout::~VgDescriptorSetLayout() {
    destroy();
}

VgDescriptorSetLayout::VgDescriptorSetLayout(VgDescriptorSetLayout&& other) noexcept {
    swap(other);
}

VgDescriptorSetLayout& VgDescriptorSetLayout::operator=(VgDescriptorSetLayout&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgDescriptorSetLayout::swap(VgDescriptorSetLayout& other) noexcept {
    std::swap(state, other.state);
}

void VgDescriptorSetLayout::destroy() {
    if (state && state->device) {
        state->device->dispose(state);
    }

    state.reset();
}

void VgDescriptorSetLayout::DescriptionSetLayoutState::destroy() {
    if (descriptorSetLayout) {
        vkDestroyDescriptorSetLayout(device->getDevice(), descriptorSetLayout, nullptr);
        descriptorSetLayout = VK_NULL_HANDLE;
    }
}
