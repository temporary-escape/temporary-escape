#include "vulkan_array_buffer.hpp"
#include "../utils/log.hpp"
#include "vulkan_renderer.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

VulkanArrayBuffer::VulkanArrayBuffer(const CreateInfo& createInfo) : createInfo{createInfo} {
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
        // logger.debug("Resizing buffer to size: {} bytes", buffer.capacity());

        // logger.warn("Creating buffer size: {}", buffer.capacity());
        VulkanBuffer::CreateInfo bufferInfo = static_cast<VulkanBuffer::CreateInfo&>(createInfo);
        bufferInfo.size = buffer.capacity();

        vbo = vulkan.createDoubleBuffer(bufferInfo);
    }

    // logger.warn("Flushing memory");
    vbo.getCurrentBuffer().subDataLocal(buffer.data(), 0, buffer.size());

    --flush;
}

void* VulkanArrayBuffer::insert(const uint64_t id) {
    auto it = indexMap.find(id);
    if (it == indexMap.end()) {
        if (buffer.size() + createInfo.stride > buffer.capacity()) {
            // logger.warn("Increasing capacity to: {}", buffer.capacity() + 1024 * createInfo.stride);
            buffer.reserve(buffer.capacity() + 1024 * createInfo.stride);
        }

        end = buffer.size();
        // logger.warn("Inserting at: {}", end);
        it = indexMap.emplace(id, end).first;
        // logger.warn("Resizing from: {} to: {}", buffer.size(), buffer.size() + createInfo.stride);
        buffer.resize(buffer.size() + createInfo.stride);
    }

    flush = MAX_FRAMES_IN_FLIGHT;
    return &buffer.at(it->second);
}

void VulkanArrayBuffer::clear() {
    indexMap.clear();
    buffer.clear();
    end = 0;
    flush = false;
}

void VulkanArrayBuffer::remove(const uint64_t id) {
    const auto it = indexMap.find(id);
    if (it != indexMap.end()) {
        flush = MAX_FRAMES_IN_FLIGHT;

        assert(buffer.size() >= createInfo.stride);

        if (end != 0) {
            // logger.warn("Copy from: {} to: {}", end, it->second);
            std::memcpy(&buffer.at(it->second), &buffer.at(end), createInfo.stride);
        }

        // logger.warn("Resizing from: {} to: {}", buffer.size(), buffer.size() - createInfo.stride);
        buffer.resize(buffer.size() - createInfo.stride);

        indexMap.erase(it);
    }
}

size_t VulkanArrayBuffer::count() const {
    if (createInfo.stride == 0) {
        return 0;
    }
    return buffer.size() / createInfo.stride;
}

size_t VulkanArrayBuffer::capacity() const {
    if (createInfo.stride == 0) {
        return 0;
    }
    return buffer.capacity() / createInfo.stride;
}

bool VulkanArrayBuffer::empty() const {
    return buffer.empty();
}
