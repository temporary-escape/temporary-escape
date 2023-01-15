#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderComponentPolyShape : public Shader {
public:
    struct Vertex {
        Vector3 position;
        Vector4 color;
    };

    struct Uniforms {
        alignas(16) Matrix4 modelMatrix;
    } __attribute__((aligned(16)));

    ShaderComponentPolyShape() = default;
    explicit ShaderComponentPolyShape(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                      VulkanRenderPass& renderPass);
    ShaderComponentPolyShape(const ShaderComponentPolyShape& other) = delete;
    ShaderComponentPolyShape(ShaderComponentPolyShape&& other) = default;
    ShaderComponentPolyShape& operator=(const ShaderComponentPolyShape& other) = delete;
    ShaderComponentPolyShape& operator=(ShaderComponentPolyShape&& other) = default;
};
} // namespace Engine
