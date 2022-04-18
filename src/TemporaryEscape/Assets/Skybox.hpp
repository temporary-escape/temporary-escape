#pragma once

#include "../Graphics/TextureCubemap.hpp"

namespace Engine {
struct Skybox {
    TextureCubemap texture{NO_CREATE};
    TextureCubemap prefilter{NO_CREATE};
    TextureCubemap irradiance{NO_CREATE};

    static Skybox createOfColor(const Color4& color) {
        static const auto apply = [](TextureCubemap& tex, const Color4& color) {
            uint8_t col[4] = {
                static_cast<uint8_t>(color.r * 255.0f),
                static_cast<uint8_t>(color.g * 255.0f),
                static_cast<uint8_t>(color.b * 255.0f),
                static_cast<uint8_t>(color.a * 255.0f),
            };

            std::unique_ptr<uint8_t[]> pixels(new uint8_t[8 * 8 * 4]);
            for (size_t i = 0; i < 8 * 8 * 4; i += 4) {
                pixels[i + 0] = col[0];
                pixels[i + 1] = col[1];
                pixels[i + 2] = col[2];
                pixels[i + 3] = col[3];
            }

            tex = TextureCubemap{};
            tex.setStorage(0, {8, 8}, PixelType::Rgba8u);
            tex.setMipMapLevel(0, 0);
            for (const auto side : TextureCubemap::sides) {
                tex.setPixels(0, {0, 0}, side, {8, 8}, PixelType::Rgba8u, pixels.get());
            }
        };

        Skybox skybox{};
        apply(skybox.texture, color);
        apply(skybox.prefilter, color);
        apply(skybox.irradiance, color);

        return skybox;
    }
};
} // namespace Engine
