#pragma once

#include "../Config.hpp"
#include "../Graphics/Canvas2D.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Library.hpp"
#include "../Utils/Packer.hpp"
#include <vector>

namespace Engine {
class ENGINE_API TextureAtlas {
public:
    explicit TextureAtlas(const Config& config, Canvas2D& canvas);

    std::tuple<Vector2i, Canvas2D::ImageHandle> add(const Vector2i& size, const void* pixels);

private:
    struct Layer {
        explicit Layer(const Config& config) : packer(2048, Vector2i{config.imageAtlasSize}) {
        }

        Texture2D texture;
        Packer packer;
        Canvas2D::ImageHandle handle{0};
    };

    const Config& config;
    Canvas2D& canvas;
    std::vector<std::shared_ptr<Layer>> layers;
};
} // namespace Engine
