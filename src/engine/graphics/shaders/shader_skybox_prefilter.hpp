#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderSkyboxPrefilter : public Shader {
public:
    struct CameraUniform {
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
    };

    struct Uniforms {
        float roughness{0};
    };

    ShaderSkyboxPrefilter() = default;
    explicit ShaderSkyboxPrefilter(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                   VulkanRenderPass& renderPass);
    ShaderSkyboxPrefilter(const ShaderSkyboxPrefilter& other) = delete;
    ShaderSkyboxPrefilter(ShaderSkyboxPrefilter&& other) = default;
    ShaderSkyboxPrefilter& operator=(const ShaderSkyboxPrefilter& other) = delete;
    ShaderSkyboxPrefilter& operator=(ShaderSkyboxPrefilter&& other) = default;
};
} // namespace Engine
