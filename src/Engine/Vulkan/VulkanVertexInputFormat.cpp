#include "VulkanVertexInputFormat.hpp"
#include "../Utils/Exceptions.hpp"

using namespace Engine;

static size_t formatToSize(const VulkanVertexInputFormat::Format format) {
    switch (format) {
    case VulkanVertexInputFormat::Format::Float: {
        return sizeof(float) * 1;
    }
    case VulkanVertexInputFormat::Format::Vec2: {
        return sizeof(float) * 2;
    }
    case VulkanVertexInputFormat::Format::Vec3: {
        return sizeof(float) * 3;
    }
    case VulkanVertexInputFormat::Format::Vec4: {
        return sizeof(float) * 4;
    }
    }
}

VulkanVertexInputFormat::VulkanVertexInputFormat(VkDevice device, const std::vector<Binding>& bindings)
    : device(device) {

    std::vector<VkVertexInputBindingDescription> bindingsDescs;
    std::vector<VkVertexInputAttributeDescription> attribDesc;

    for (const auto& bind : bindings) {
        // Create VulkanEZ vertex input format.
        bindingsDescs.push_back(VkVertexInputBindingDescription{bind.index, 0, VK_VERTEX_INPUT_RATE_VERTEX});

        size_t totalSize = 0;

        for (const auto& attr : bind.attributes) {
            auto& desc = attribDesc.emplace_back();
            desc.format = static_cast<VkFormat>(attr.format);
            desc.location = attr.location;
            desc.binding = attr.binding;
            desc.offset = totalSize;

            totalSize += formatToSize(attr.format);
        }

        bindingsDescs.back().stride = totalSize;
    }

    VezVertexInputFormatCreateInfo vertexInputFormatCreateInfo = {};
    vertexInputFormatCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingsDescs.size());
    vertexInputFormatCreateInfo.pVertexBindingDescriptions = bindingsDescs.data();
    vertexInputFormatCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDesc.size());
    vertexInputFormatCreateInfo.pVertexAttributeDescriptions = attribDesc.data();

    if (vezCreateVertexInputFormat(device, &vertexInputFormatCreateInfo, &format) != VK_SUCCESS) {
        EXCEPTION("Failed to create vertex input format (vezCreateVertexInputFormat)");
    }
}

VulkanVertexInputFormat::~VulkanVertexInputFormat() {
    reset();
}

VulkanVertexInputFormat::VulkanVertexInputFormat(VulkanVertexInputFormat&& other) noexcept {
    swap(other);
}

VulkanVertexInputFormat& VulkanVertexInputFormat::operator=(VulkanVertexInputFormat&& other) noexcept {
    if (this != &other) {
        swap(other);
    }

    return *this;
}

void VulkanVertexInputFormat::swap(VulkanVertexInputFormat& other) noexcept {
    std::swap(format, other.format);
    std::swap(device, other.device);
}

void VulkanVertexInputFormat::reset() {
    if (device && format) {
        vezDestroyVertexInputFormat(device, format);
    }
    device = VK_NULL_HANDLE;
    format = VK_NULL_HANDLE;
}
