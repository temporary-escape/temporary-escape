#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderComponentDebug : public Shader {
public:
    struct Vertex {
        Vector3 position;
        Vector4 color;
    };

    struct Uniforms {
        Matrix4 modelMatrix;
    } __attribute__((aligned(16)));

    ShaderComponentDebug() = default;
    explicit ShaderComponentDebug(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                  VulkanRenderPass& renderPass);
    ShaderComponentDebug(const ShaderComponentDebug& other) = delete;
    ShaderComponentDebug(ShaderComponentDebug&& other) = default;
    ShaderComponentDebug& operator=(const ShaderComponentDebug& other) = delete;
    ShaderComponentDebug& operator=(ShaderComponentDebug&& other) = default;
};
} // namespace Engine
