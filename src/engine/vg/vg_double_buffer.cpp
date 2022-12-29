#include "vg_double_buffer.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgDoubleBuffer::VgDoubleBuffer(const Config& config, VgDevice& device, const CreateInfo& createInfo) : device{&device} {
    for (auto& buffer : buffers) {
        buffer = device.createBuffer(createInfo);
    }
}

VgDoubleBuffer::~VgDoubleBuffer() {
    destroy();
}

VgDoubleBuffer::VgDoubleBuffer(VgDoubleBuffer&& other) noexcept {
    swap(other);
}

VgDoubleBuffer& VgDoubleBuffer::operator=(VgDoubleBuffer&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgDoubleBuffer::swap(VgDoubleBuffer& other) noexcept {
    std::swap(device, other.device);
    std::swap(buffers, other.buffers);
}

void VgDoubleBuffer::destroy() {
    for (auto& buffer : buffers) {
        buffer.destroy();
    }
    device = nullptr;
}

void VgDoubleBuffer::subData(const void* data, size_t offset, size_t size) {
    getCurrentBuffer().subData(data, offset, size);
}

const VgBuffer& VgDoubleBuffer::getCurrentBuffer() const {
    if (buffers.size() == 1) {
        return buffers.back();
    }
    return buffers.at(device->getCurrentFrameNum());
}

VgBuffer& VgDoubleBuffer::getCurrentBuffer() {
    if (buffers.size() == 1) {
        return buffers.back();
    }
    return buffers.at(device->getCurrentFrameNum());
}
