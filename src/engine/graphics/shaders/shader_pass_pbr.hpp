#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderPassPbr : public Shader {
public:
    static constexpr size_t maxDirectionalLights = 4;

    struct Vertex {
        Vector2 position;
    };

    struct DirectionalLightsUniform {
        Vector4 colors[maxDirectionalLights];
        Vector4 directions[maxDirectionalLights];
        int count{0};
    };

    ShaderPassPbr() = default;
    explicit ShaderPassPbr(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                           VulkanRenderPass& renderPass);
    ShaderPassPbr(const ShaderPassPbr& other) = delete;
    ShaderPassPbr(ShaderPassPbr&& other) = default;
    ShaderPassPbr& operator=(const ShaderPassPbr& other) = delete;
    ShaderPassPbr& operator=(ShaderPassPbr&& other) = default;
};
} // namespace Engine
