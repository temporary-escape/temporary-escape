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
};
} // namespace Engine
