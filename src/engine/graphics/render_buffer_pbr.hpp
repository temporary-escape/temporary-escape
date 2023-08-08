#pragma once

#include "render_buffer.hpp"

namespace Engine {
class ENGINE_API RenderBufferPbr : public RenderBuffer {
public:
    static constexpr size_t bloomMipMaps = 5;

    enum Attachment : uint32_t {
        Depth = 0,
        Forward,
        AlbedoAmbient,
        EmissiveRoughness,
        NormalMetallic,
        ShadowL0,
        ShadowL1,
        ShadowL2,
        ShadowL3,
        Entity,
        SSAO,
        FXAA,
        BloomL0,
        BloomL1,
        BloomL2,
        BloomL3,
        BloomL4,
        Max = UINT32_MAX,
    };

    explicit RenderBufferPbr(const RenderOptions& options, VulkanRenderer& vulkan);
    NON_MOVEABLE(RenderBufferPbr);
    NON_COPYABLE(RenderBufferPbr);
};
} // namespace Engine
