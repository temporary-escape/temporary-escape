#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderComponentLines : public Shader {
public:
    struct Vertex {
        Vector3 position;
        Color4 color;
    };

    struct Uniforms {
        Matrix4 modelMatrix;
    } __attribute__((aligned(16)));

    ShaderComponentLines() = default;
    explicit ShaderComponentLines(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                  VulkanRenderPass& renderPass);
    ShaderComponentLines(const ShaderComponentLines& other) = delete;
    ShaderComponentLines(ShaderComponentLines&& other) = default;
    ShaderComponentLines& operator=(const ShaderComponentLines& other) = delete;
    ShaderComponentLines& operator=(ShaderComponentLines&& other) = default;
};
} // namespace Engine
