#include "vg_uniform_buffer.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgUniformBuffer::VgUniformBuffer(const Config& config, VgDevice& device, const size_t size,
                                 const VgUniformBuffer::Usage usage) :
    device{&device} {

    VgBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY;

    if (usage == Usage::Static) {
        buffers.push_back(device.createBuffer(bufferInfo));
    } else {
        buffers.push_back(device.createBuffer(bufferInfo));
        buffers.push_back(device.createBuffer(bufferInfo));
    }
}

VgUniformBuffer::~VgUniformBuffer() {
    destroy();
}

VgUniformBuffer::VgUniformBuffer(VgUniformBuffer&& other) noexcept {
    swap(other);
}

VgUniformBuffer& VgUniformBuffer::operator=(VgUniformBuffer&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgUniformBuffer::swap(VgUniformBuffer& other) noexcept {
    std::swap(device, other.device);
    std::swap(buffers, other.buffers);
}

void VgUniformBuffer::destroy() {
    for (auto& buffer : buffers) {
        buffer.destroy();
    }
    buffers.clear();
    device = nullptr;
}

void VgUniformBuffer::subData(const void* data, size_t offset, size_t size) {
    getCurrentBuffer().subData(data, offset, size);
}

const VgBuffer& VgUniformBuffer::getCurrentBuffer() const {
    if (buffers.size() == 1) {
        return buffers.back();
    }
    return buffers.at(device->getCurrentFrameNum());
}

VgBuffer& VgUniformBuffer::getCurrentBuffer() {
    if (buffers.size() == 1) {
        return buffers.back();
    }
    return buffers.at(device->getCurrentFrameNum());
}
