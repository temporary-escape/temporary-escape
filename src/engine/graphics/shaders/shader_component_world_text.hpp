#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderComponentWorldText : public Shader {
public:
    struct Vertex {
        Vector3 position;
        Vector2 offset;
        Vector2 size;
        Vector2 uv;
        Vector2 st;
    };

    struct Uniforms {
        alignas(16) Matrix4 modelMatrix;
        alignas(16) Color4 color;
    } __attribute__((aligned(16)));

    ShaderComponentWorldText() = default;
    explicit ShaderComponentWorldText(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                      VulkanRenderPass& renderPass);
    ShaderComponentWorldText(const ShaderComponentWorldText& other) = delete;
    ShaderComponentWorldText(ShaderComponentWorldText&& other) = default;
    ShaderComponentWorldText& operator=(const ShaderComponentWorldText& other) = delete;
    ShaderComponentWorldText& operator=(ShaderComponentWorldText&& other) = default;
};
} // namespace Engine
