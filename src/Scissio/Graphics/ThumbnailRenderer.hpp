#pragma once

#include "../Assets/Model.hpp"
#include "Renderer.hpp"
#include "SkyboxRenderer.hpp"

namespace Scissio {
class ThumbnailRenderer {
public:
    explicit ThumbnailRenderer(const Config& config, SkyboxRenderer& skyboxRenderer);
    virtual ~ThumbnailRenderer() = default;

    void render(Renderer& renderer, const ModelPtr& model);
    Vector2i getSize() const;
    std::unique_ptr<char[]> getPixels();

private:
    const Config& config;
    GBuffer gBuffer;
    FBuffer fBuffer;
    Skybox skybox;
};
} // namespace Scissio