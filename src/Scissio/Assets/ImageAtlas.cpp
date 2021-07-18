#define STB_RECT_PACK_IMPLEMENTATION
#include "ImageAtlas.hpp"
#include "../Utils/Exceptions.hpp"
#include <stb_rect_pack.h>

using namespace Scissio;

ImageAtlas::ImageAtlas(const Config& config) : config(config), size(config.imageAtlasSize) {
}

ImageAtlas::~ImageAtlas() = default;

std::shared_ptr<ImageNode> ImageAtlas::reserve(const Vector2i& size) {
    Log::d("Image atlas reserving new size: {}x{}", size.x, size.y);

    for (auto& layer : layers) {
        const auto pos = layer->reserve(size);
        if (pos.has_value()) {
            auto ptr = std::make_shared<ImageNode>(ImageNode{pos.value(), size, &layer->getTexture()});
            imageNodes.push_back(ptr);
            return ptr;
        }
    }

    layers.emplace_back(new Layer(config));
    const auto& layer = layers.back();

    const auto pos = layer->reserve(size);
    if (!pos.has_value()) {
        EXCEPTION("Unable to reserve image of size: {}x{} atlas out of size", size.x, size.y);
    }

    auto ptr = std::make_shared<ImageNode>(ImageNode{pos.value(), size, &layer->getTexture()});
    imageNodes.push_back(ptr);
    return ptr;
}

ImageAtlas::Layer::Layer(const Config& config) : ctx(new stbrp_context{}), nodes(new stbrp_node[4096]) {
    Log::w("Image atlas allocating new layer size: {}x{} with 4096 nodes", config.imageAtlasSize,
           config.imageAtlasSize);
    stbrp_init_target(ctx.get(), config.imageAtlasSize, config.imageAtlasSize, nodes.get(), 4096);

    texture.setStorage(0, Vector2i{config.imageAtlasSize}, PixelType::Rgba8u);
}

ImageAtlas::Layer::~Layer() = default;

std::optional<Vector2i> ImageAtlas::Layer::reserve(const Vector2i& size) {
    stbrp_rect rect;
    rect.w = size.x;
    rect.h = size.y;
    const auto ret = stbrp_pack_rects(ctx.get(), &rect, 1);
    if (ret > 0) {
        return {Vector2i{rect.x, rect.y}};
    } else {
        return {std::nullopt};
    }
}
