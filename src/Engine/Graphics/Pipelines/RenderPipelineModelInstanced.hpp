#pragma once

#include "../RenderPipeline.hpp"

namespace Engine {
class ENGINE_API RenderPipelineModelInstanced : public RenderPipeline {
public:
    explicit RenderPipelineModelInstanced(VulkanRenderer& vulkan);
};
} // namespace Engine
