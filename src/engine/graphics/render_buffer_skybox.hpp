#pragma once

#include "render_buffer.hpp"

namespace Engine {
class ENGINE_API RenderBufferSkybox : public RenderBuffer {
public:
    enum Attachment : uint32_t {
        Color = 0,
        Max = UINT32_MAX,
    };

    explicit RenderBufferSkybox(const RenderOptions& options, VulkanRenderer& vulkan);
    NON_MOVEABLE(RenderBufferSkybox);
    NON_COPYABLE(RenderBufferSkybox);
};
} // namespace Engine
