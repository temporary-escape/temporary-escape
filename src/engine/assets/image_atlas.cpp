#include "image_atlas.hpp"
#include "../utils/exceptions.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ImageAtlas::Layer::Layer(const Config& config, VulkanRenderer& vulkan) :
    vulkan{vulkan}, packer{2048, Vector2i{config.graphics.imageAtlasSize}} {

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VK_FORMAT_R8G8B8A8_SRGB;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {
        static_cast<uint32_t>(config.graphics.imageAtlasSize),
        static_cast<uint32_t>(config.graphics.imageAtlasSize),
        1,
    };
    textureInfo.image.mipLevels = getMipMapLevels({config.graphics.imageAtlasSize, config.graphics.imageAtlasSize});
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = VK_FORMAT_R8G8B8A8_SRGB;
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
    textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);

    texture = vulkan.createTexture(textureInfo);
    vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    auto pixels = std::make_unique<char[]>(config.graphics.imageAtlasSize * config.graphics.imageAtlasSize * 4);
    std::memset(pixels.get(), 0x00, config.graphics.imageAtlasSize * config.graphics.imageAtlasSize * 4);
    vulkan.copyDataToImage(
        texture, 0, {0, 0}, 0, {config.graphics.imageAtlasSize, config.graphics.imageAtlasSize}, pixels.get());
}

std::optional<Vector2i> ImageAtlas::Layer::add(const Vector2i& size, const void* pixels) {
    const auto res = packer.add(size + Vector2i{8, 8});
    if (!res) {
        return std::nullopt;
    }

    const auto pos = *res + Vector2i{4, 4};

    vulkan.copyDataToImage(texture, 0, pos, 0, size, pixels);
    return pos;
}

std::optional<Vector2i> ImageAtlas::Layer::add(const Vector2i& size, const VulkanTexture& source) {
    const auto res = packer.add(size + Vector2i{8, 8});
    if (!res) {
        return std::nullopt;
    }

    const auto pos = *res + Vector2i{4, 4};

    vulkan.copyImageToImage(texture, 0, pos, 0, size, source);

    return pos;
}

void ImageAtlas::Layer::finalize() {
    // vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vulkan.generateMipMaps(texture);
}

ImageAtlas::ImageAtlas(const Config& config, VulkanRenderer& vulkan) : config{&config}, vulkan{&vulkan} {
}

ImageAtlas::Allocation ImageAtlas::add(const Vector2i& size, const void* pixels) {
    return add(size, [&](Layer& layer) { return layer.add(size, pixels); });
}

ImageAtlas::Allocation ImageAtlas::add(const Vector2i& size, const VulkanTexture& source) {
    return add(size, [&](Layer& layer) { return layer.add(size, source); });
}

ImageAtlas::Allocation ImageAtlas::add(const Vector2i& size, const ImageAtlas::LayerAddFunc& provider) {
    Vector2i pos;
    Allocation allocation{};
    allocation.size = size;
    allocation.st = {
        static_cast<float>(size.x) / static_cast<float>(config->graphics.imageAtlasSize),
        static_cast<float>(size.y) / static_cast<float>(config->graphics.imageAtlasSize),
    };

    for (auto& layer : layers) {
        auto res = provider(*layer);
        if (res) {
            allocation.pos = *res;
            allocation.uv = {
                static_cast<float>(res->x) / static_cast<float>(config->graphics.imageAtlasSize),
                static_cast<float>(res->y) / static_cast<float>(config->graphics.imageAtlasSize),
            };

            allocation.texture = &layer->getTexture();

            return allocation;
        }
    }

    layers.emplace_back(std::make_unique<Layer>(*config, *vulkan));
    auto& layer = layers.back();

    auto res = provider(*layer);
    if (!res) {
        EXCEPTION("Unable to allocate image of size: {} in image atlas", size);
    }

    allocation.pos = *res;
    allocation.uv = {
        static_cast<float>(res->x) / static_cast<float>(config->graphics.imageAtlasSize),
        static_cast<float>(res->y) / static_cast<float>(config->graphics.imageAtlasSize),
    };

    allocation.texture = &layer->getTexture();

    return allocation;
}

void ImageAtlas::finalize() {
    for (auto& layer : layers) {
        layer->finalize();
    }
}
