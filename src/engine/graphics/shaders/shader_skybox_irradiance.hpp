#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderSkyboxIrradiance : public Shader {
public:
    struct CameraUniform {
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
    };

    ShaderSkyboxIrradiance() = default;
    explicit ShaderSkyboxIrradiance(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                    VulkanRenderPass& renderPass);
    ShaderSkyboxIrradiance(const ShaderSkyboxIrradiance& other) = delete;
    ShaderSkyboxIrradiance(ShaderSkyboxIrradiance&& other) = default;
    ShaderSkyboxIrradiance& operator=(const ShaderSkyboxIrradiance& other) = delete;
    ShaderSkyboxIrradiance& operator=(ShaderSkyboxIrradiance&& other) = default;
};
} // namespace Engine
