#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderPassFXAA : public Shader {
public:
    static constexpr size_t maxDirectionalLights = 4;

    struct Vertex {
        Vector2 position;
    };

    struct Uniforms {
        Vector2 textureSize;
    };

    ShaderPassFXAA() = default;
    explicit ShaderPassFXAA(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                            VulkanRenderPass& renderPass);
    ShaderPassFXAA(const ShaderPassFXAA& other) = delete;
    ShaderPassFXAA(ShaderPassFXAA&& other) = default;
    ShaderPassFXAA& operator=(const ShaderPassFXAA& other) = delete;
    ShaderPassFXAA& operator=(ShaderPassFXAA&& other) = default;
};
} // namespace Engine
