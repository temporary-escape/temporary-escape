#pragma once

#include "../Assets/AssetManager.hpp"
#include "Renderer.hpp"

namespace Engine {
class ENGINE_API Imager {
public:
    explicit Imager(const Config& config, AssetManager& assetManager, Renderer& renderer);

    void createThumbnail(const AssetBlockPtr& block);

private:
    void render(Scene& scene, const Vector2i& viewport);

    const Config& config;
    AssetManager& assetManager;
    Renderer& renderer;
    GBuffer gBuffer;
};
} // namespace Engine
