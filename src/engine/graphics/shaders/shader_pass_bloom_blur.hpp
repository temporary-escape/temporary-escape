#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderPassBloomBlur : public Shader {
public:
    struct Vertex {
        Vector2 position;
    };

    struct GaussianWeightsUniform {
        Vector4 weight[32];
        int count{0};
    };

    struct ALIGNED(16) Uniforms {
        bool horizontal{false};
    };

    static_assert(sizeof(Uniforms) == 16);

    ShaderPassBloomBlur() = default;
    explicit ShaderPassBloomBlur(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                 VulkanRenderPass& renderPass);
    ShaderPassBloomBlur(const ShaderPassBloomBlur& other) = delete;
    ShaderPassBloomBlur(ShaderPassBloomBlur&& other) = default;
    ShaderPassBloomBlur& operator=(const ShaderPassBloomBlur& other) = delete;
    ShaderPassBloomBlur& operator=(ShaderPassBloomBlur&& other) = default;
};
} // namespace Engine
