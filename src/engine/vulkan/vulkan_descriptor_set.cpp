#include "vulkan_descriptor_set.hpp"
#include "../utils/exceptions.hpp"
#include "vulkan_descriptor_pool.hpp"
#include "vulkan_descriptor_set_layout.hpp"
#include "vulkan_device.hpp"

using namespace Engine;

VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device, VulkanDescriptorPool& descriptorPool,
                                         VulkanDescriptorSetLayout& layout) :
    device{device} {

    auto layoutPtr = layout.getHandle();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool.getHandle();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layoutPtr;

    if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
        EXCEPTION("Failed to allocate descriptor sets!");
    }
}

VulkanDescriptorSet::~VulkanDescriptorSet() = default;

VulkanDescriptorSet::VulkanDescriptorSet(VulkanDescriptorSet&& other) noexcept {
    swap(other);
}

VulkanDescriptorSet& VulkanDescriptorSet::operator=(VulkanDescriptorSet&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VulkanDescriptorSet::swap(VulkanDescriptorSet& other) noexcept {
    std::swap(descriptorSet, other.descriptorSet);
    std::swap(device, other.device);
}

void VulkanDescriptorSet::bind(const Span<VulkanBufferBinding>& uniforms, const Span<VulkanTextureBinding>& textures) {

    std::vector<VkDescriptorBufferInfo> bufferInfos{uniforms.size()};
    std::vector<VkDescriptorImageInfo> imageInfos{textures.size()};

    for (size_t i = 0; i < uniforms.size(); i++) {
        bufferInfos[i].buffer = uniforms[i].uniform->getHandle();
        bufferInfos[i].offset = 0;
        bufferInfos[i].range = uniforms[i].uniform->getSize();
    }

    for (size_t i = 0; i < textures.size(); i++) {
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[i].imageView = textures[i].texture->getImageView();
        imageInfos[i].sampler = textures[i].texture->getSampler();
    }

    std::vector<VkWriteDescriptorSet> writes{uniforms.size() + textures.size()};

    for (size_t i = 0; i < uniforms.size(); i++) {
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = descriptorSet;
        writes[i].dstBinding = uniforms[i].binding;
        writes[i].dstArrayElement = 0;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[i].descriptorCount = 1;
        writes[i].pBufferInfo = &bufferInfos.at(i);
    }

    for (size_t i = 0, w = uniforms.size(); i < textures.size(); w++, i++) {
        writes[w].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[w].dstSet = descriptorSet;
        writes[w].dstBinding = textures[i].binding;
        writes[w].dstArrayElement = 0;
        writes[w].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[w].descriptorCount = 1;
        writes[w].pImageInfo = &imageInfos.at(i);
    }

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}
