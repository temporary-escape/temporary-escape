#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderPassDebug : public Shader {
public:
    struct Vertex {
        Vector2 position;
    };

    ShaderPassDebug() = default;
    explicit ShaderPassDebug(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                             VulkanRenderPass& renderPass);
    ShaderPassDebug(const ShaderPassDebug& other) = delete;
    ShaderPassDebug(ShaderPassDebug&& other) = default;
    ShaderPassDebug& operator=(const ShaderPassDebug& other) = delete;
    ShaderPassDebug& operator=(ShaderPassDebug&& other) = default;
};
} // namespace Engine
