#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderPassSSAO : public Shader {
public:
    struct Vertex {
        Vector2 position;
    };

    struct Uniforms {
        Vector2 scale;
    };

    struct SamplesUniform {
        Vector4 weights[64];
    };

    ShaderPassSSAO() = default;
    explicit ShaderPassSSAO(const Config& config, VulkanRenderer& vulkan);
    ShaderPassSSAO(const ShaderPassSSAO& other) = delete;
    ShaderPassSSAO(ShaderPassSSAO&& other) = default;
    ShaderPassSSAO& operator=(const ShaderPassSSAO& other) = delete;
    ShaderPassSSAO& operator=(ShaderPassSSAO&& other) = default;
    void finalize(VulkanRenderPass& renderPass) override;

private:
    VulkanRenderer* vulkan{nullptr};
    VulkanShaderModule vert;
    VulkanShaderModule frag;
};
} // namespace Engine
