#include "MaterialTextures.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static VkFormat getFormatTargetETC(TextureCompressionTarget target) {
    switch (target) {
    case TextureCompressionTarget::R: {
        return VkFormat::VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    }
    case TextureCompressionTarget::RG: {
        return VkFormat::VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    }
    case TextureCompressionTarget::RGB: {
        return VkFormat::VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    }
    case TextureCompressionTarget::RGBA: {
        return VkFormat::VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    }
    default: {
        EXCEPTION("Unknown compression target");
    }
    }
}

static VkFormat getFormatTargetBC(TextureCompressionTarget target) {
    switch (target) {
    case TextureCompressionTarget::R: {
        return VkFormat::VK_FORMAT_BC4_UNORM_BLOCK;
    }
    case TextureCompressionTarget::RG: {
        return VkFormat::VK_FORMAT_BC5_UNORM_BLOCK;
    }
    case TextureCompressionTarget::RGB: {
        return VkFormat::VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    }
    case TextureCompressionTarget::RGBA: {
        return VkFormat::VK_FORMAT_BC3_UNORM_BLOCK;
    }
    default: {
        EXCEPTION("Unknown compression target");
    }
    }
}

static VkFormat getFormatTargetDefault(TextureCompressionTarget target) {
    (void)target;
    return VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
}

static VkFormat getFormatTarget(VulkanCompressionType type, TextureCompressionTarget target) {
    switch (type) {
    case VulkanCompressionType::BC3: {
        return getFormatTargetBC(target);
    }
    case VulkanCompressionType::ETC2: {
        return getFormatTargetETC(target);
    }
    default: {
        return getFormatTargetDefault(target);
    }
    }
}

MaterialTextures::TextureArray::TextureArray(VulkanRenderer& vulkan, const Vector2i& size,
                                             const TextureCompressionTarget target, const size_t count) :
    vulkan{vulkan}, size{size}, count{count}, next{0} {

    const auto format = getFormatTarget(vulkan.getCompressionType(), target);

    VkExtent3D extent{
        static_cast<uint32_t>(size.x),
        static_cast<uint32_t>(size.y),
        1,
    };

    const auto numMipMaps = static_cast<int>(std::log2(size.x)) + 1;

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = format;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    // textureInfo.image.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    textureInfo.image.extent = extent;
    textureInfo.image.mipLevels = numMipMaps;
    textureInfo.image.arrayLayers = count;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = format;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = numMipMaps;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = count;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;

    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    if (vulkan.getPhysicalDeviceFeatures().samplerAnisotropy) {
        textureInfo.sampler.anisotropyEnable = VK_TRUE;
        textureInfo.sampler.maxAnisotropy =
            std::max(4.0f, vulkan.getPhysicalDeviceProperties().limits.maxSamplerAnisotropy);
    } else {
        textureInfo.sampler.anisotropyEnable = VK_FALSE;
        textureInfo.sampler.maxAnisotropy = 1.0f;
    }

    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);

    texture = vulkan.createTexture(textureInfo);

    vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}

MaterialTextures::TextureArray::~TextureArray() {
    if (texture) {
        vulkan.dispose(std::move(texture));
    }
}

int MaterialTextures::TextureArray::addLayer() {
    if (next + 1 > count) {
        EXCEPTION("Unable to add next texture array layer, error: out of bounds");
    }
    return next++;
}

void MaterialTextures::TextureArray::finalize() {
    vulkan.transitionImageLayout(
        texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

MaterialTextures::MaterialTextures(VulkanRenderer& vulkan, const Vector2i& size, const Counts& counts) :
    diffuse{vulkan, size, TextureCompressionTarget::RGBA, counts.at(TextureUsage::Diffuse)},
    emissive{vulkan, size, TextureCompressionTarget::RGB, counts.at(TextureUsage::Emissive)},
    normal{vulkan, size, TextureCompressionTarget::RG, counts.at(TextureUsage::Normal)},
    metallicRoughness{vulkan, size, TextureCompressionTarget::RGB, counts.at(TextureUsage::MetallicRoughness)},
    ambientOcclusion{vulkan, size, TextureCompressionTarget::R, counts.at(TextureUsage::AmbientOcclusion)},
    mask{vulkan, size, TextureCompressionTarget::R, counts.at(TextureUsage::Mask)} {
}

int MaterialTextures::addLayer(const TextureUsage usage) {
    return get(usage).addLayer();
}

MaterialTextures::TextureArray& MaterialTextures::get(const TextureUsage usage) {
    switch (usage) {
    case TextureUsage::Diffuse: {
        return diffuse;
    }
    case TextureUsage::Emissive: {
        return emissive;
    }
    case TextureUsage::Normal: {
        return normal;
    }
    case TextureUsage::MetallicRoughness: {
        return metallicRoughness;
    }
    case TextureUsage::AmbientOcclusion: {
        return ambientOcclusion;
    }
    case TextureUsage::Mask: {
        return mask;
    }
    default: {
        EXCEPTION("Unknown texture usage of type: {}", static_cast<int>(usage));
    }
    }
}

const MaterialTextures::TextureArray& MaterialTextures::get(const TextureUsage usage) const {
    switch (usage) {
    case TextureUsage::Diffuse: {
        return diffuse;
    }
    case TextureUsage::Emissive: {
        return emissive;
    }
    case TextureUsage::Normal: {
        return normal;
    }
    case TextureUsage::MetallicRoughness: {
        return metallicRoughness;
    }
    case TextureUsage::AmbientOcclusion: {
        return ambientOcclusion;
    }
    case TextureUsage::Mask: {
        return mask;
    }
    default: {
        EXCEPTION("Unknown texture usage of type: {}", static_cast<int>(usage));
    }
    }
}

void MaterialTextures::finalize() {
    diffuse.finalize();
    emissive.finalize();
    normal.finalize();
    metallicRoughness.finalize();
    ambientOcclusion.finalize();
    mask.finalize();
}
