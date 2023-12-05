#include "ComponentCamera.hpp"
#include "ComponentTransform.hpp"

using namespace Engine;

// TODO
static const Config config{};

ComponentCamera::ComponentCamera(entt::registry& reg, entt::entity handle, ComponentTransform& transform) :
    Component{reg, handle}, Camera{transform.getTransform()} {
}

void ComponentCamera::update(const float delta) {
}

void ComponentCamera::recalculate(VulkanRenderer& vulkan, const Vector2i& viewport) {
    setViewport(viewport);
    auto uniform = createUniform(false);

    if (!ubo) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(Uniform);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        bufferInfo.memoryFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        ubo = vulkan.createDoubleBuffer(bufferInfo);
    }

    ubo.subDataLocal(&uniform, 0, sizeof(Uniform));

    uniform = createUniform(true);

    if (!uboZeroPos) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(Uniform);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        bufferInfo.memoryFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        uboZeroPos = vulkan.createDoubleBuffer(bufferInfo);
    }

    uboZeroPos.subDataLocal(&uniform, 0, sizeof(Uniform));
}
