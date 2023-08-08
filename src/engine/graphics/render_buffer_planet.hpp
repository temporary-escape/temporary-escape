#pragma once

#include "render_buffer.hpp"

namespace Engine {
class ENGINE_API RenderBufferPlanet : public RenderBuffer {
public:
    enum Attachment : uint32_t {
        Heightmap = 0,
        Moisture = 1,
        Color = 2,
        MetallicRoughness = 3,
        Normal = 4,
        Max = UINT32_MAX,
    };

    explicit RenderBufferPlanet(const RenderOptions& options, VulkanRenderer& vulkan);
    NON_MOVEABLE(RenderBufferPlanet);
    NON_COPYABLE(RenderBufferPlanet);
};
} // namespace Engine
