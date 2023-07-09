#pragma once

#include "render_pass.hpp"
#include "render_subpass_opaque.hpp"

namespace Engine {
class ENGINE_API RenderPassOpaque : public RenderPass {
public:
    enum Attachments {
        Depth = 0,
        AlbedoAmbient = 1,
        EmissiveRoughness = 2,
        NormalMetallic = 3,
        Entity = 4,
    };

    static const size_t totalAttachments = 5;

    explicit RenderPassOpaque(VulkanRenderer& vulkan, RenderResources& resources, AssetsManager& assetsManager,
                              const Vector2i& viewport, VoxelShapeCache& voxelShapeCache, const VulkanTexture& depth);
    virtual ~RenderPassOpaque() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

    void setMousePos(const Vector2i& value) {
        mousePos = value;
    }

    uint32_t getMousePosEntity();

private:
    RenderSubpassOpaque subpassOpaque;
    VulkanDoubleBuffer entityColorBuffer;
    Vector2i mousePos;
};
} // namespace Engine
