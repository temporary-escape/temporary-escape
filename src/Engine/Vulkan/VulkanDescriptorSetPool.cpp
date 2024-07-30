#include "VulkanDescriptorSetPool.hpp"
#include "../Utils/Exceptions.hpp"
#include "VulkanDevice.hpp"

using namespace Engine;

VulkanDescriptorSetPool::VulkanDescriptorSetPool(VulkanDevice& device, const Span<Binding>& bindings,
                                                 const size_t maxSets) :
    device{&device} {

    layout = VulkanDescriptorSetLayout{device, bindings};

    std::unordered_map<VkDescriptorType, uint32_t> descriptorTypeCounts;
    for (const auto& binding : bindings) {
        descriptorTypeCounts[binding.descriptorType] += binding.descriptorCount;
    }

    VulkanDescriptorPool::CreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

    std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
    descriptorPoolSizes.resize(descriptorTypeCounts.size());
    size_t idx = 0;
    for (const auto& [type, count] : descriptorTypeCounts) {
        descriptorPoolSizes[idx].type = type;
        descriptorPoolSizes[idx].descriptorCount = count * maxSets;
        idx++;
    }

    poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * maxSets;

    pool = VulkanDescriptorPool{device, poolInfo};
}

VulkanDescriptorSetPool::~VulkanDescriptorSetPool() {
    VulkanDescriptorSetPool::destroy();
}

VulkanDescriptorSetPool::VulkanDescriptorSetPool(VulkanDescriptorSetPool&& other) noexcept {
    swap(other);
}

VulkanDescriptorSetPool& VulkanDescriptorSetPool::operator=(VulkanDescriptorSetPool&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanDescriptorSetPool::swap(VulkanDescriptorSetPool& other) noexcept {
    std::swap(pool, other.pool);
    std::swap(layout, other.layout);
    std::swap(device, other.device);
}

void VulkanDescriptorSetPool::destroy() {
    layout.destroy();
    pool.destroy();
}

VulkanDescriptorSet VulkanDescriptorSetPool::createDescriptorSet() {
    return VulkanDescriptorSet{device->getDevice(), pool, layout, false};
}
