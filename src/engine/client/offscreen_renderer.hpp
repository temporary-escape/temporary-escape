#pragma once

#include "../assets/block.hpp"
#include "../graphics/renderer.hpp"

namespace Engine {
class ENGINE_API OffscreenRenderer {
public:
    explicit OffscreenRenderer(const Config& config, const Vector2i& viewport, VulkanRenderer& vulkan, Canvas& canvas,
                               Nuklear& nuklear, ShaderModules& shaderModules, VoxelShapeCache& voxelShapeCache,
                               VoxelPalette& voxelPalette, FontFamily& font);
    ~OffscreenRenderer() = default;

    void render(const std::shared_ptr<Block>& block, VoxelShape::Type shape);

    [[nodiscard]] const VulkanTexture& getTexture() const {
        return fboColor;
    }

    [[nodiscard]] const Vector2i& getViewport() const {
        return viewport;
    }

private:
    void createRenderPass(VulkanRenderer& vulkan);
    void createFramebuffer(VulkanRenderer& vulkan, const Vector2i& viewport);
    void createTexture(VulkanRenderer& vulkan, const Vector2i& viewport);

    const Vector2i viewport;
    VulkanRenderer& vulkan;
    VulkanFramebuffer fbo;
    VulkanTexture fboColor;
    VulkanRenderPass renderPass;
    Skybox skybox;
    std::shared_ptr<Renderer> renderer;
};
} // namespace Engine
