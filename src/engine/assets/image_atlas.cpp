#include "image_atlas.hpp"
#include "../utils/exceptions.hpp"

#define CMP "ImageAtlas"

using namespace Engine;

ImageAtlas::Layer::Layer(const Config& config, VulkanRenderer& vulkan) : packer{2048, Vector2i{config.imageAtlasSize}} {
    /*VulkanTexture::Descriptor desc{};
    desc.levels = 1;
    desc.type = VulkanTexture::Type::VK_IMAGE_TYPE_2D;
    desc.format = VulkanTexture::Format::VK_FORMAT_R8G8B8A8_UNORM;
    desc.size = {config.imageAtlasSize, config.imageAtlasSize};
    texture = vulkan.createTexture(desc);

    auto pixels = std::unique_ptr<char[]>(new char[config.imageAtlasSize * config.imageAtlasSize * 4]);
    std::memset(pixels.get(), 0x00, config.imageAtlasSize * config.imageAtlasSize * 4);

    texture.subData(0, {0, 0}, {config.imageAtlasSize, config.imageAtlasSize}, pixels.get());*/
}

std::optional<Vector2i> ImageAtlas::Layer::add(const Vector2i& size, const void* pixels) {
    const auto res = packer.add(size);
    if (!res) {
        return std::nullopt;
    }

    // texture.subData(0, *res, size, pixels);
    return res;
}

ImageAtlas::ImageAtlas(const Config& config, VulkanRenderer& vulkan) : config{config}, vulkan{vulkan} {
}

ImageAtlas::Allocation ImageAtlas::add(const Vector2i& size, const void* pixels) {
    Vector2i pos;
    Allocation allocation{};
    allocation.size = size;
    allocation.st = {
        static_cast<float>(size.x) / static_cast<float>(config.imageAtlasSize),
        static_cast<float>(size.y) / static_cast<float>(config.imageAtlasSize),
    };

    for (auto& layer : layers) {
        auto res = layer->add(size, pixels);
        if (res) {
            allocation.pos = *res;
            allocation.uv = {
                static_cast<float>(res->x) / static_cast<float>(config.imageAtlasSize),
                static_cast<float>(res->y) / static_cast<float>(config.imageAtlasSize),
            };

            allocation.texture = &layer->getTexture();

            return allocation;
        }
    }

    layers.push_back(std::make_unique<Layer>(config, vulkan));
    auto& layer = layers.back();

    auto res = layer->add(size, pixels);
    if (!res) {
        EXCEPTION("Unable to allocate image of size: {} in image atlas", size);
    }

    allocation.pos = *res;
    allocation.uv = {
        static_cast<float>(res->x) / static_cast<float>(config.imageAtlasSize),
        static_cast<float>(res->y) / static_cast<float>(config.imageAtlasSize),
    };

    allocation.texture = &layer->getTexture();

    return allocation;
}
