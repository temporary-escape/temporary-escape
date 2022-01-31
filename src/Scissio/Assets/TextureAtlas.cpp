#include "TextureAtlas.hpp"
#include "../Utils/Exceptions.hpp"

using namespace Scissio;

TextureAtlas::TextureAtlas(const Config& config, Canvas2D& canvas) : config(config), canvas(canvas) {
}

std::tuple<Vector2i, Canvas2D::ImageHandle> TextureAtlas::add(const Vector2i& size, const void* pixels) {
    if (size.x > config.imageAtlasSize || size.y > config.imageAtlasSize) {
        EXCEPTION("Image of size: [{}, {}] is too large for texture atlas", size.x, size.y);
    }

    Texture2D* texture = nullptr;
    Canvas2D::ImageHandle handle = 0;
    Vector2i pos = {0, 0};

    for (auto& layer : layers) {
        auto result = layer->packer.add(size);
        if (result.has_value()) {
            pos = result.value();
            texture = &layer->texture;
            handle = layer->handle;
            break;
        }
    }

    layers.push_back(std::make_shared<Layer>(config));
    layers.back()->texture.setStorage(0, Vector2i{config.imageAtlasSize}, PixelType::Rgba8u);
    layers.back()->handle = canvas.textureToImageHandle(layers.back()->texture, Vector2i{config.imageAtlasSize});

    auto result = layers.back()->packer.add(size);
    if (result.has_value()) {
        pos = result.value();
        texture = &layers.back()->texture;
        handle = layers.back()->handle;
    }

    if (!texture) {
        EXCEPTION("Unable to add image of size: [{}, {}] into texture atlas", size.x, size.y);
    }

    texture->setPixels(0, pos, size, PixelType::Rgba8u, pixels);

    return {pos, handle};
}
