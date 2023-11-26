#pragma once

#include "RenderBuffer.hpp"

namespace Engine {
class ENGINE_API RenderBufferSkybox : public RenderBuffer {
public:
    enum Attachment : uint32_t {
        Color = 0,
        Irradiance = 1,
        Prefilter = 2,
        Max = UINT32_MAX,
    };

    explicit RenderBufferSkybox(const Config& config, VulkanRenderer& vulkan);
    NON_MOVEABLE(RenderBufferSkybox);
    NON_COPYABLE(RenderBufferSkybox);
};
} // namespace Engine
