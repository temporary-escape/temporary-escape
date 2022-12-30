#include "vg_sync_object.hpp"
#include "../utils/exceptions.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgSyncObject::VgSyncObject(const Config& config, VgDevice& device) : device{&device} {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {

        destroy();
        EXCEPTION("Failed to create synchronization objects for a frame!");
    }
}

VgSyncObject::~VgSyncObject() {
    destroy();
}

VgSyncObject::VgSyncObject(VgSyncObject&& other) noexcept {
    swap(other);
}

VgSyncObject& VgSyncObject::operator=(VgSyncObject&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgSyncObject::swap(VgSyncObject& other) noexcept {
    std::swap(device, other.device);
    std::swap(imageAvailableSemaphore, other.imageAvailableSemaphore);
    std::swap(renderFinishedSemaphore, other.renderFinishedSemaphore);
    std::swap(inFlightFence, other.inFlightFence);
}

void VgSyncObject::destroy() {
    if (imageAvailableSemaphore) {
        vkDestroySemaphore(device->getDevice(), imageAvailableSemaphore, nullptr);
        imageAvailableSemaphore = VK_NULL_HANDLE;
    }

    if (renderFinishedSemaphore) {
        vkDestroySemaphore(device->getDevice(), renderFinishedSemaphore, nullptr);
        renderFinishedSemaphore = VK_NULL_HANDLE;
    }

    if (inFlightFence) {
        vkDestroyFence(device->getDevice(), inFlightFence, nullptr);
        inFlightFence = VK_NULL_HANDLE;
    }
}

void VgSyncObject::reset() {
    vkResetFences(device->getDevice(), 1, &inFlightFence);
}

void VgSyncObject::wait() {
    vkWaitForFences(device->getDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
}
