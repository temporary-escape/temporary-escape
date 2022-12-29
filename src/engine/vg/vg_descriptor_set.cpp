#include "vg_descriptor_set.hpp"
#include "../utils/exceptions.hpp"
#include "vg_device.hpp"

using namespace Engine;

VgDescriptorSet::VgDescriptorSet(const Config& config, VgDevice& device, VgDescriptorPool& descriptorPool,
                                 VgDescriptorSetLayout& layout) :
    device{device.getHandle()} {

    auto layoutPtr = layout.getHandle();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool.getHandle();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layoutPtr;

    if (vkAllocateDescriptorSets(device.getHandle(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
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

void VgDescriptorSet::bind(const std::vector<VgUniformBufferBinding>& uniforms) {
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    bufferInfos.resize(uniforms.size());

    for (size_t i = 0; i < uniforms.size(); i++) {
        bufferInfos[i].buffer = uniforms.at(i).uniform->getHandle();
        bufferInfos[i].offset = 0;
        bufferInfos[i].range = uniforms.at(i).uniform->getSize();
    }

    std::vector<VkWriteDescriptorSet> writes;
    writes.resize(uniforms.size());

    for (size_t i = 0; i < uniforms.size(); i++) {
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = descriptorSet;
        writes[i].dstBinding = 0;
        writes[i].dstArrayElement = 0;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[i].descriptorCount = 1;
        writes[i].pBufferInfo = &bufferInfos.at(i);
    }

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}
