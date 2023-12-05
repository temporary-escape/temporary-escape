#include "RenderBuffer.hpp"
#include "../Utils/Exceptions.hpp"

using namespace Engine;

RenderBuffer::RenderBuffer(VulkanRenderer& vulkan) : vulkan{vulkan} {
}

RenderBuffer::~RenderBuffer() {
    for (auto& texture : textures) {
        texture.destroy();
    }
}

VulkanTexture& RenderBuffer::createTexture(const TextureInfo& info) {
    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = info.format;
    textureInfo.image.imageType = info.type;
    textureInfo.image.extent = {static_cast<uint32_t>(info.size.x), static_cast<uint32_t>(info.size.y), 1};
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = info.layers;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = info.usage | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = info.format;
    textureInfo.view.viewType = info.viewType;
    textureInfo.view.subresourceRange.aspectMask = info.aspectMask; // VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = info.layers;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.addressModeU = info.addressMode;
    textureInfo.sampler.addressModeV = info.addressMode;
    textureInfo.sampler.addressModeW = info.addressMode;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = info.borderColor;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureInfo.sampler.minLod = 0.0f;
    textureInfo.sampler.maxLod = 0.0f;
    textureInfo.sampler.borderColor = info.borderColor;

    textures.push_back(vulkan.createTexture(textureInfo));
    return textures.back();
}

void RenderBuffer::createAttachment(const uint32_t attachment, VulkanTexture& texture,
                                    const RenderBuffer::ViewInfo& info) {
    VulkanImageView::CreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType = info.type;
    viewInfo.format = texture.getFormat();
    viewInfo.subresourceRange = {};
    viewInfo.subresourceRange.aspectMask = info.aspectMask;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = info.arrayLayer;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.image = texture.getHandle();

    attachments.at(attachment) = {&texture, vulkan.createImageView(viewInfo)};
}

const VulkanTexture& RenderBuffer::getAttachmentTexture(const uint32_t attachment) const {
    const auto& found = attachments.at(attachment);
    if (found.texture == nullptr) {
        EXCEPTION("Attachment: {} has null texture", attachment);
    }
    return *found.texture;
}