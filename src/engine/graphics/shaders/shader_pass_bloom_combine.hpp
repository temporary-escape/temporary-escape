#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderPassBloomCombine : public Shader {
public:
    struct Vertex {
        Vector2 position;
    };

    struct Uniforms {
        float strength;
        float exposure;
        float gamma;
    } __attribute__((aligned(16)));

    ShaderPassBloomCombine() = default;
    explicit ShaderPassBloomCombine(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                    VulkanRenderPass& renderPass);
    ShaderPassBloomCombine(const ShaderPassBloomCombine& other) = delete;
    ShaderPassBloomCombine(ShaderPassBloomCombine&& other) = default;
    ShaderPassBloomCombine& operator=(const ShaderPassBloomCombine& other) = delete;
    ShaderPassBloomCombine& operator=(ShaderPassBloomCombine&& other) = default;
};
} // namespace Engine
