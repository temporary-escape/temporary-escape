#include "vg_texture.hpp"
#include "../utils/exceptions.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgTexture::VgTexture(const Config& config, VgDevice& device, const CreateInfo& createInfo) :
    state{std::make_shared<BufferState>()} {

    state->device = &device;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.flags = 0;

    VmaAllocationInfo allocationInfo;
    if (vmaCreateImage(device.getAllocator(), &createInfo.image, &allocInfo, &state->image, &state->allocation,
                       &allocationInfo) != VK_SUCCESS) {
        destroy();
        EXCEPTION("Failed to allocate image memory!");
    }

    auto createInfoSampler = createInfo.sampler;
    createInfoSampler.maxAnisotropy = device.getPhysicalDeviceProperties().limits.maxSamplerAnisotropy;

    if (vkCreateSampler(device.getHandle(), &createInfoSampler, nullptr, &state->sampler) != VK_SUCCESS) {
        destroy();
        EXCEPTION("Failed to allocate image sampler!");
    }

    auto createInfoView = createInfo.view;
    createInfoView.image = state->image;

    if (vkCreateImageView(device.getHandle(), &createInfoView, nullptr, &state->view) != VK_SUCCESS) {
        EXCEPTION("Failed to allocate image view!");
    }

    device.transitionImageLayout(*this, createInfo.image.format, VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    state->format = createInfo.image.format;
    state->extent = createInfo.image.extent;
}

VgTexture::~VgTexture() {
    destroy();
}

VgTexture::VgTexture(VgTexture&& other) noexcept {
    swap(other);
}

VgTexture& VgTexture::operator=(VgTexture&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgTexture::swap(VgTexture& other) noexcept {
    std::swap(state, other.state);
}

void VgTexture::destroy() {
    if (state && state->device) {
        state->device->dispose(state);
    }

    state.reset();
}

VkDeviceSize VgTexture::getDataSize() const {
    return getFormatDataSize(state->format, state->extent);
}

void VgTexture::subData(int level, const Vector2i& offset, int layer, const Vector2i& size, const void* data) {
    auto region = VkExtent3D{static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1};

    VgBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = getFormatDataSize(state->format, region);
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY;

    auto stagingBuffer = state->device->createBuffer(bufferInfo);
    stagingBuffer.subData(data, 0, bufferInfo.size);

    state->device->copyBufferToImage(stagingBuffer, *this, level, layer, {offset.x, offset.y, 0}, region);

    state->device->transitionImageLayout(*this, state->format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VgTexture::BufferState::destroy() {
    if (sampler) {
        vkDestroySampler(device->getHandle(), sampler, nullptr);
        sampler = VK_NULL_HANDLE;
    }

    if (view) {
        vkDestroyImageView(device->getHandle(), view, nullptr);
        view = VK_NULL_HANDLE;
    }

    if (image) {
        vmaDestroyImage(device->getAllocator(), image, allocation);
        image = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }
}
