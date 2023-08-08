#pragma once

#include "render_buffer.hpp"

namespace Engine {
class ENGINE_API RenderBufferPbr : public RenderBuffer {
public:
    enum Attachment : uint32_t {
        Depth = 0,
        Forward,
        Max = UINT32_MAX,
    };

    explicit RenderBufferPbr(const RenderOptions& options, VulkanRenderer& vulkan);
    NON_MOVEABLE(RenderBufferPbr);
    NON_COPYABLE(RenderBufferPbr);
};
} // namespace Engine
