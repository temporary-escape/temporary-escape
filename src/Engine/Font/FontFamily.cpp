#include "FontFamily.hpp"
#include "TextShaper.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Vector2 Engine::textAlignToVector(TextAlign textAlign) {
    switch (textAlign) {
    case TextAlign::Left: {
        return {0.0f, 0.5f};
    }
    case TextAlign::Center: {
        return {0.5f, 0.5f};
    }
    case TextAlign::Right: {
        return {1.0f, 0.5f};
    }
    case TextAlign::LeftTop: {
        return {0.0f, 0.0f};
    }
    case TextAlign::CenterTop: {
        return {0.5f, 0.0f};
    }
    case TextAlign::RightTop: {
        return {1.0f, 0.0f};
    }
    case TextAlign::LeftBottom: {
        return {0.0f, 1.0f};
    }
    case TextAlign::CenterBottom: {
        return {0.5f, 1.0f};
    }
    case TextAlign::RightBottom: {
        return {1.0f, 1.0f};
    }
    default: {
        return {0.0f, 0.0f};
    }
    }
}

FontFamily::FontFamily(VulkanRenderer& vulkan, const Sources& sources, const int size) : size{size} {
    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VK_FORMAT_R8_UNORM;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {static_cast<uint32_t>(atlasSize.x), static_cast<uint32_t>(atlasSize.y), 1};
    textureInfo.image.mipLevels = getMipMapLevels(atlasSize);
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = VK_FORMAT_R8_UNORM;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 1;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureInfo.sampler.minLod = 0.0f;
    textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);

    texture = vulkan.createTexture(textureInfo);

    vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    Vector2i offset{0, 0};
    for (size_t i = 0; i < faces.size(); i++) {
        if (offset.y + FontFace::atlasSize.y >= FontFamily::atlasSize.y) {
            EXCEPTION("Font atlas of size: {} is too small", FontFamily::atlasSize);
        }

        faces[i] = std::make_unique<FontFace>(vulkan, sources[i], texture, offset, size);

        offset.x += FontFace::atlasSize.x;
        if (offset.x >= FontFamily::atlasSize.x) {
            offset.x = 0;
            offset.y += FontFace::atlasSize.y;
        }
    }

    vulkan.generateMipMaps(texture);

    logger.info("Loaded font of size: {} with atlas size: {}", size, atlasSize);
}

float FontFamily::getGlyphAdvance(const uint32_t code, const float height) const {
    return faces[static_cast<size_t>(FontType::Regular)]->getGlyphAdvance(code, height);
}

Vector2 FontFamily::getBounds(const std::string_view& text, const float height) const {
    TextShaper shaper{*this, height};
    shaper.write(text);
    return shaper.getBounds();
}

Vector2 FontFamily::getPenOffset(const std::string_view& text, const float height, const TextAlign textAlign) const {
    return getBounds(text, height) * textAlignToVector(textAlign);
}

std::string FontFamily::wrapText(const std::string_view& text, const float height, const float width) const {
    TextWrapper wrapper{*this, height, width};
    wrapper.write(text);
    wrapper.flush(text.data() + text.size());
    return wrapper.getResult();
}
