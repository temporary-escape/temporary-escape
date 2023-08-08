#pragma once

#include "../../assets/texture.hpp"
#include "../render_buffer_pbr.hpp"
#include "../render_pass.hpp"

namespace Engine {
class ENGINE_API RenderPassForward : public RenderPass {
public:
    explicit RenderPassForward(VulkanRenderer& vulkan, RenderBufferPbr& buffer, RenderResources& resources,
                               AssetsManager& assetsManager);

    void beforeRender(VulkanCommandBuffer& vkb) override;
    void render(VulkanCommandBuffer& vkb, Scene& scene) override;

private:
    RenderBufferPbr& buffer;
    RenderResources& resources;
};
} // namespace Engine
