#pragma once

#include "../math/matrix.hpp"
#include "shader.hpp"

namespace Engine {
class ShaderComponentGrid : public Shader {
public:
    struct Vertex {
        Vector3 position;
        Vector3 normal;
        Vector2 texCoords;
        Vector4 tangent;
    };

    struct Uniforms {
        Matrix4 modelMatrix;
        Matrix3 normalMatrix;
    };

    ShaderComponentGrid() = default;
    explicit ShaderComponentGrid(const Config& config, VulkanRenderer& vulkan);
    ShaderComponentGrid(const ShaderComponentGrid& other) = delete;
    ShaderComponentGrid(ShaderComponentGrid&& other) = default;
    ShaderComponentGrid& operator=(const ShaderComponentGrid& other) = delete;
    ShaderComponentGrid& operator=(ShaderComponentGrid&& other) = default;
    void finalize(VulkanRenderPass& renderPass) override;

private:
    VulkanRenderer* vulkan{nullptr};
    VulkanShaderModule vert;
    VulkanShaderModule frag;
    VulkanShaderModule geom;
};
} // namespace Engine
