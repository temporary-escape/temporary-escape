#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderSkyboxNebula : public Shader {
public:
    struct CameraUniform {
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
    };

    struct Uniforms {
        Vector4 uColor;
        Vector4 uOffset;
        float uScale;
        float uIntensity;
        float uFalloff;
    };

    ShaderSkyboxNebula() = default;
    explicit ShaderSkyboxNebula(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                VulkanRenderPass& renderPass);
    ShaderSkyboxNebula(const ShaderSkyboxNebula& other) = delete;
    ShaderSkyboxNebula(ShaderSkyboxNebula&& other) = default;
    ShaderSkyboxNebula& operator=(const ShaderSkyboxNebula& other) = delete;
    ShaderSkyboxNebula& operator=(ShaderSkyboxNebula&& other) = default;
};
} // namespace Engine
