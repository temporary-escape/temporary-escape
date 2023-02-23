#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderPassBloomExtract : public Shader {
public:
    struct Vertex {
        Vector2 position;
    };

    struct ALIGNED(16) Uniforms {
        float threshold{0.5f};
    };

    static_assert(sizeof(Uniforms) == 16);

    ShaderPassBloomExtract() = default;
    explicit ShaderPassBloomExtract(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                    VulkanRenderPass& renderPass);
    ShaderPassBloomExtract(const ShaderPassBloomExtract& other) = delete;
    ShaderPassBloomExtract(ShaderPassBloomExtract&& other) = default;
    ShaderPassBloomExtract& operator=(const ShaderPassBloomExtract& other) = delete;
    ShaderPassBloomExtract& operator=(ShaderPassBloomExtract&& other) = default;
};
} // namespace Engine
