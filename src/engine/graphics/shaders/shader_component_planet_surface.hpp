#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderComponentPlanetSurface : public Shader {
public:
    struct Vertex {
        Vector3 position;
        Vector3 normal;
    };

    struct ALIGNED(16) Uniforms {
        Matrix4 modelMatrix;
        Matrix3 normalMatrix;
    };

    ShaderComponentPlanetSurface() = default;
    explicit ShaderComponentPlanetSurface(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                          VulkanRenderPass& renderPass);
    ShaderComponentPlanetSurface(const ShaderComponentPlanetSurface& other) = delete;
    ShaderComponentPlanetSurface(ShaderComponentPlanetSurface&& other) = default;
    ShaderComponentPlanetSurface& operator=(const ShaderComponentPlanetSurface& other) = delete;
    ShaderComponentPlanetSurface& operator=(ShaderComponentPlanetSurface&& other) = default;
};
} // namespace Engine
