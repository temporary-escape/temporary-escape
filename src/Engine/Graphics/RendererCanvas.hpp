#pragma once

#include "../Vulkan/VulkanRenderer.hpp"
#include "Canvas2.hpp"

namespace Engine {
class RendererCanvas {
public:
    explicit RendererCanvas(VulkanRenderer& vulkan);

    void reset();
    void render(VulkanCommandBuffer& vkb, Canvas2& canvas, const Vector2i& viewport);

private:
    void create();
    void createDefaultTexture();

    VulkanRenderer& vulkan;
    VulkanShader shaderFrag;
    VulkanShader shaderVert;
    VulkanTexture defaultTexture;
    VulkanDescriptorSetLayout descriptorSetLayout;
    std::array<VulkanDescriptorPool, MAX_FRAMES_IN_FLIGHT> descriptorPools;
    VulkanPipeline pipeline;
};
} // namespace Engine
