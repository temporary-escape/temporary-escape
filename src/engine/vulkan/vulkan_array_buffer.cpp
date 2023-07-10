#include "vulkan_array_buffer.hpp"
#include "../utils/log.hpp"
#include "vulkan_renderer.hpp"

using namespace Engine;

// static auto logger = createLogger(LOG_FILENAME);

VulkanArrayBuffer::VulkanArrayBuffer(const size_t stride) : stride{stride} {
}

VulkanArrayBuffer::~VulkanArrayBuffer() {
    VulkanArrayBuffer::destroy();
}

void VulkanArrayBuffer::destroy() {
    vbo.destroy();
}

void VulkanArrayBuffer::recalculate(VulkanRenderer& vulkan) {
    if (flush == 0) {
        return;
    }

    if (!vbo || vbo.getSize() != buffer.capacity()) {
        // logger.warn("Creating buffer size: {}", buffer.capacity());
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = buffer.capacity();
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        bufferInfo.memoryFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vbo = vulkan.createDoubleBuffer(bufferInfo);
    }

    // logger.warn("Flushing memory");
    vbo.getCurrentBuffer().subDataLocal(buffer.data(), 0, buffer.size());

    --flush;
}

void* VulkanArrayBuffer::insert(const uint64_t id) {
    auto it = indexMap.find(id);
    if (it == indexMap.end()) {
        if (buffer.size() + stride > buffer.capacity()) {
            // logger.warn("Increasing capacity to: {}", buffer.capacity() + 1024 * stride);
            buffer.reserve(buffer.capacity() + 1024 * stride);
        }

        end = buffer.size();
        // logger.warn("Inserting at: {}", end);
        it = indexMap.emplace(id, end).first;
        // logger.warn("Resizing from: {} to: {}", buffer.size(), buffer.size() + stride);
        buffer.resize(buffer.size() + stride);
    }

    flush = MAX_FRAMES_IN_FLIGHT;
    return &buffer.at(it->second);
}

void VulkanArrayBuffer::remove(const uint64_t id) {
    const auto it = indexMap.find(id);
    if (it != indexMap.end()) {
        flush = MAX_FRAMES_IN_FLIGHT;

        assert(buffer.size() >= stride);

        if (end != 0) {
            // logger.warn("Copy from: {} to: {}", end, it->second);
            std::memcpy(&buffer.at(it->second), &buffer.at(end), stride);
        }

        // logger.warn("Resizing from: {} to: {}", buffer.size(), buffer.size() - stride);
        buffer.resize(buffer.size() - stride);
    }
}

size_t VulkanArrayBuffer::count() const {
    return buffer.size() / stride;
}
