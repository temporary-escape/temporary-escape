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
    };

    static const size_t totalAttachments = 4;

    explicit RenderPassOpaque(VulkanRenderer& vulkan, Registry& registry, const Vector2i& viewport,
                              VoxelShapeCache& voxelShapeCache, const VulkanTexture& depth);
    virtual ~RenderPassOpaque() = default;

    void render(VulkanCommandBuffer& vkb, const Vector2i& viewport, Scene& scene);

private:
    RenderSubpassOpaque subpassOpaque;
};
} // namespace Engine
