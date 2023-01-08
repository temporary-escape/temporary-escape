#pragma once

#include "../shader.hpp"

namespace Engine {
class ShaderBrdf : public Shader {
public:
    struct Vertex {
        Vector2 pos;
    };

    ShaderBrdf() = default;
    explicit ShaderBrdf(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                        VulkanRenderPass& renderPass);
    ShaderBrdf(const ShaderBrdf& other) = delete;
    ShaderBrdf(ShaderBrdf&& other) = default;
    ShaderBrdf& operator=(const ShaderBrdf& other) = delete;
    ShaderBrdf& operator=(ShaderBrdf&& other) = default;
};
} // namespace Engine
