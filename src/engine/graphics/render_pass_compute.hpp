#pragma once

#include "render_pass.hpp"
#include "render_subpass_compute.hpp"

namespace Engine {
class ENGINE_API RenderPassCompute : public RenderPass {
public:
    explicit RenderPassCompute(VulkanRenderer& vulkan, Registry& registry);
    virtual ~RenderPassCompute() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene);

private:
    RenderSubpassCompute subpassCompute;
};
} // namespace Engine
