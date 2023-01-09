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
        float bloomStrength;
        float bloomPower;
        float exposure;
        float gamma;
        float contrast;
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
