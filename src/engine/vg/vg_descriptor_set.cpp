#include "vg_descriptor_set.hpp"
#include "../utils/exceptions.hpp"
#include "vg_renderer.hpp"

using namespace Engine;

VgDescriptorSet::VgDescriptorSet(const Config& config, VgRenderer& device, VgDescriptorPool& descriptorPool,
                                 VgDescriptorSetLayout& layout) :
    device{device.getDevice()} {

    auto layoutPtr = layout.getHandle();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool.getHandle();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layoutPtr;

    if (vkAllocateDescriptorSets(device.getDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
        EXCEPTION("Failed to allocate descriptor sets!");
    }
}

VgDescriptorSet::~VgDescriptorSet() = default;

VgDescriptorSet::VgDescriptorSet(VgDescriptorSet&& other) noexcept {
    swap(other);
}

VgDescriptorSet& VgDescriptorSet::operator=(VgDescriptorSet&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void VgDescriptorSet::swap(VgDescriptorSet& other) noexcept {
    std::swap(descriptorSet, other.descriptorSet);
    std::swap(device, other.device);
}

void VgDescriptorSet::bind(const std::vector<VgBufferBinding>& uniforms,
                           const std::vector<VgTextureBinding>& textures) {

    std::vector<VkDescriptorBufferInfo> bufferInfos{uniforms.size()};
    std::vector<VkDescriptorImageInfo> imageInfos{textures.size()};

    for (size_t i = 0; i < uniforms.size(); i++) {
        bufferInfos[i].buffer = uniforms.at(i).uniform->getHandle();
        bufferInfos[i].offset = 0;
        bufferInfos[i].range = uniforms.at(i).uniform->getSize();
    }

    for (size_t i = 0; i < textures.size(); i++) {
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[i].imageView = textures.at(i).texture->getImageView();
        imageInfos[i].sampler = textures.at(i).texture->getSampler();
    }

    std::vector<VkWriteDescriptorSet> writes{uniforms.size() + textures.size()};

    for (size_t i = 0; i < uniforms.size(); i++) {
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = descriptorSet;
        writes[i].dstBinding = uniforms.at(i).binding;
        writes[i].dstArrayElement = 0;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[i].descriptorCount = 1;
        writes[i].pBufferInfo = &bufferInfos.at(i);
    }

    for (size_t i = 0, w = uniforms.size(); i < textures.size(); w++, i++) {
        writes[w].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[w].dstSet = descriptorSet;
        writes[w].dstBinding = textures.at(i).binding;
        writes[w].dstArrayElement = 0;
        writes[w].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[w].descriptorCount = 1;
        writes[w].pImageInfo = &imageInfos.at(i);
    }

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}
