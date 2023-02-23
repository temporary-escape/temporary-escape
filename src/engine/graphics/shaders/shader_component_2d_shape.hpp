#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderComponent2DShape : public Shader {
public:
    struct Vertex {
        Vector3 position;
    };

    struct ALIGNED(16) Uniforms {
        Matrix4 modelMatrix;
        Color4 color;
    };

    ShaderComponent2DShape() = default;
    explicit ShaderComponent2DShape(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules,
                                    VulkanRenderPass& renderPass);
    ShaderComponent2DShape(const ShaderComponent2DShape& other) = delete;
    ShaderComponent2DShape(ShaderComponent2DShape&& other) = default;
    ShaderComponent2DShape& operator=(const ShaderComponent2DShape& other) = delete;
    ShaderComponent2DShape& operator=(ShaderComponent2DShape&& other) = default;
};
} // namespace Engine
