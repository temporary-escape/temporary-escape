#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderPassSkybox : public Shader {
public:
    struct Vertex {
        Vector3 position;
    };

    struct Uniforms {
        Matrix4 modelMatrix;
    };

    ShaderPassSkybox() = default;
    explicit ShaderPassSkybox(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                              VulkanRenderPass& renderPass);
    ShaderPassSkybox(const ShaderPassSkybox& other) = delete;
    ShaderPassSkybox(ShaderPassSkybox&& other) = default;
    ShaderPassSkybox& operator=(const ShaderPassSkybox& other) = delete;
    ShaderPassSkybox& operator=(ShaderPassSkybox&& other) = default;
};
} // namespace Engine
