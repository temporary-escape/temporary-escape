#include "VulkanDescriptorSet.hpp"
#include "../Utils/Exceptions.hpp"
#include "VulkanDescriptorPool.hpp"
#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanDevice.hpp"

using namespace Engine;

VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device, const VulkanDescriptorPool& descriptorPool,
                                         const VulkanDescriptorSetLayout& layout) :
    device{device} {

    auto layoutPtr = layout.getHandle();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool.getHandle();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layoutPtr;

    if (const auto err = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet); err != VK_SUCCESS) {
        EXCEPTION("Failed to allocate descriptor sets! Error: {}", err);
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

void VulkanDescriptorSet::bind(const Span<VkWriteDescriptorSet>& writes) {
    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void VulkanDescriptorSet::bind(const Span<VulkanBufferBinding>& uniforms, const Span<VulkanTextureBinding>& textures,
                               const Span<VulkanTextureBinding>& inputAttachments) {

    std::vector<VkDescriptorBufferInfo> bufferInfos{uniforms.size()};
    std::vector<VkDescriptorImageInfo> imageInfos{textures.size() + inputAttachments.size()};

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

    for (size_t i = 0, w = textures.size(); i < inputAttachments.size(); w++, i++) {
        imageInfos[w].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[w].imageView = inputAttachments[i].texture->getImageView();
        imageInfos[w].sampler = inputAttachments[i].texture->getSampler();
    }

    std::vector<VkWriteDescriptorSet> writes{uniforms.size() + textures.size() + inputAttachments.size()};

    for (size_t i = 0; i < uniforms.size(); i++) {
        if (uniforms[i].uniform->getDescriptorType() == VK_DESCRIPTOR_TYPE_MAX_ENUM) {
            EXCEPTION("Unable to use this buffer for descriptor set");
        }

        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = descriptorSet;
        writes[i].dstBinding = uniforms[i].binding;
        writes[i].dstArrayElement = 0;
        writes[i].descriptorType = uniforms[i].uniform->getDescriptorType();
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

    for (size_t i = 0, w = uniforms.size() + textures.size(); i < inputAttachments.size(); w++, i++) {
        writes[w].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[w].dstSet = descriptorSet;
        writes[w].dstBinding = inputAttachments[i].binding;
        writes[w].dstArrayElement = 0;
        writes[w].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        writes[w].descriptorCount = 1;
        writes[w].pImageInfo = &imageInfos.at(textures.size() + i);
    }

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}
