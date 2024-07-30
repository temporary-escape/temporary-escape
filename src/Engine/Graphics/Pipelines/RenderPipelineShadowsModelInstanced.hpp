#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineShadowsModelInstanced : public RenderPipeline {
public:
    explicit RenderPipelineShadowsModelInstanced(VulkanRenderer& vulkan);
};
} // namespace Engine
