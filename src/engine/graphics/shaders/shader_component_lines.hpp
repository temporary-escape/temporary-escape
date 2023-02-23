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

    struct ALIGNED(16) Uniforms {
        Matrix4 modelMatrix;
        Color4 color;
    };

    ShaderComponentLines() = default;
    explicit ShaderComponentLines(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                  VulkanRenderPass& renderPass);
    ShaderComponentLines(const ShaderComponentLines& other) = delete;
    ShaderComponentLines(ShaderComponentLines&& other) = default;
    ShaderComponentLines& operator=(const ShaderComponentLines& other) = delete;
    ShaderComponentLines& operator=(ShaderComponentLines&& other) = default;
};
} // namespace Engine
