#include "ThumbnailRenderer.hpp"
#include "../Scene/Camera.hpp"
#include "../Scene/ComponentModel.hpp"
#include "../Scene/Scene.hpp"

using namespace Scissio;

static const std::array<GLenum, 6> CubemapEnums = {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

static std::unique_ptr<uint8_t[]> pixelsFromColor(const Vector2i& size, const Color4& color) {
    std::unique_ptr<uint8_t[]> pixels(new uint8_t[size.x * size.y * 3]);
    for (size_t i = 0; i < size.x * size.y * 3; i += 3) {
        pixels[i + 0] = static_cast<uint8_t>(color.r * 255.0f);
        pixels[i + 1] = static_cast<uint8_t>(color.g * 255.0f);
        pixels[i + 2] = static_cast<uint8_t>(color.b * 255.0f);
    }
    return pixels;
}

ThumbnailRenderer::ThumbnailRenderer(const Config& config, SkyboxRenderer& skyboxRenderer)
    : config{config}, fBuffer(true) {
    TextureCubemap texture;
    texture.bind();

    const auto pixels = pixelsFromColor({64, 64}, {0.07f, 0.07f, 0.07f, 1.0f});
    for (const auto& side : CubemapEnums) {
        glTexImage2D(side, 0, GL_RGB8, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
    }

    texture.texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture.texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture.texParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    texture.texParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    texture.texParameteri(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    texture.texParameteri(GL_TEXTURE_BASE_LEVEL, 0);
    texture.texParameteri(GL_TEXTURE_MAX_LEVEL, 0);

    auto prefilter = skyboxRenderer.prefilter(texture);
    auto irradiance = skyboxRenderer.irradiance(texture);

    skybox = Skybox{
        std::move(texture),
        std::move(prefilter),
        std::move(irradiance),
    };
}

void ThumbnailRenderer::render(Renderer& renderer, const ModelPtr& model) {
    const auto viewport = getSize();

    Camera camera;

    camera.translate({0.0f, 0.0f, 2.0f});
    camera.rotate({0.0f, 1.0f, 0.0f}, 45.0f);
    camera.rotate({1.0f, 0.0f, 0.0f}, -37.0f);
    camera.move({2.0f, 2.0f, 2.0f});
    camera.setProjection(viewport, 30.0f);

    Scene scene;

    auto entity = scene.addEntity();
    entity->addComponent<ComponentModel>(model);
    entity->rotate({0.0f, 1.0f, 0.0f}, 180.0f);

    scene.update();

    renderer.setViewport(viewport);
    renderer.setGBuffer(gBuffer);
    renderer.setFBuffer(fBuffer);
    renderer.setSkybox(skybox);
    renderer.setView(camera.getViewMatrix());
    renderer.setProjection(camera.getProjectionMatrix());

    RenderOptions options;
    options.withSkybox = true;

    renderer.render(scene, options);
}

Vector2i ThumbnailRenderer::getSize() const {
    return Vector2i{config.thumbnailSize};
}

std::unique_ptr<char[]> ThumbnailRenderer::getPixels() {
    const auto viewport = getSize();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fBuffer.getFbo().getHandle());
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    std::unique_ptr<char[]> pixels(new char[viewport.x * viewport.y * 4]);
    glReadPixels(0, 0, viewport.x, viewport.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());

    return pixels;
}
