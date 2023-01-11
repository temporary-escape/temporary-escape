#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderComponentPointCloud : public Shader {
public:
    struct Vertex {
        Vector3 position;
        Vector2 size;
        Vector4 color;
        Vector2 uv;
        Vector2 st;
    };

    struct Uniforms {
        Matrix4 modelMatrix;
    } __attribute__((aligned(16)));

    ShaderComponentPointCloud() = default;
    explicit ShaderComponentPointCloud(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                       VulkanRenderPass& renderPass);
    ShaderComponentPointCloud(const ShaderComponentPointCloud& other) = delete;
    ShaderComponentPointCloud(ShaderComponentPointCloud&& other) = default;
    ShaderComponentPointCloud& operator=(const ShaderComponentPointCloud& other) = delete;
    ShaderComponentPointCloud& operator=(ShaderComponentPointCloud&& other) = default;
};
} // namespace Engine
