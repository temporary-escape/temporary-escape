#pragma once

#include "shader.hpp"

namespace Engine {
class ShaderBrdf : public Shader {
public:
    struct Vertex {
        Vector2 pos;
    };

    ShaderBrdf() = default;
    explicit ShaderBrdf(const Config& config, VulkanRenderer& vulkan);
    ShaderBrdf(const ShaderBrdf& other) = delete;
    ShaderBrdf(ShaderBrdf&& other) = default;
    ShaderBrdf& operator=(const ShaderBrdf& other) = delete;
    ShaderBrdf& operator=(ShaderBrdf&& other) = default;
    void finalize(VulkanRenderPass& renderPass) override;

private:
    VulkanRenderer* vulkan{nullptr};
    VulkanShaderModule vert;
    VulkanShaderModule frag;
};
} // namespace Engine
