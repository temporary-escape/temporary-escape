#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderSkyboxStars : public Shader {
public:
    struct Vertex {
        Vector3 position;
        float brightness;
        Color4 color;
    };

    struct CameraUniform {
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
    };

    struct Uniforms {
        Vector2 particleSize;
    };

    ShaderSkyboxStars() = default;
    explicit ShaderSkyboxStars(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                               VulkanRenderPass& renderPass);
    ShaderSkyboxStars(const ShaderSkyboxStars& other) = delete;
    ShaderSkyboxStars(ShaderSkyboxStars&& other) = default;
    ShaderSkyboxStars& operator=(const ShaderSkyboxStars& other) = delete;
    ShaderSkyboxStars& operator=(ShaderSkyboxStars&& other) = default;
};
} // namespace Engine
