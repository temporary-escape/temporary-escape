#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderPassFlare : public Shader {
public:
    struct Vertex {
        Vector3 position;
    };

    struct ALIGNED(16) Uniforms {
        Vector2 size;
        float temp{0.0f};
    };

    ShaderPassFlare() = default;
    explicit ShaderPassFlare(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                             VulkanRenderPass& renderPass);
    ShaderPassFlare(const ShaderPassFlare& other) = delete;
    ShaderPassFlare(ShaderPassFlare&& other) = default;
    ShaderPassFlare& operator=(const ShaderPassFlare& other) = delete;
    ShaderPassFlare& operator=(ShaderPassFlare&& other) = default;
};
} // namespace Engine
