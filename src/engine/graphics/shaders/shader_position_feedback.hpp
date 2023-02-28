#pragma once

#include "../../math/matrix.hpp"
#include "../shader.hpp"

namespace Engine {
class ShaderPositionFeedback : public Shader {
public:
    struct Uniforms {
        alignas(16) Matrix4 modelmatrix;
        alignas(16) Vector2 viewport;
        alignas(8) int count{0};
    };

    ShaderPositionFeedback() = default;
    explicit ShaderPositionFeedback(const Config& config, VulkanRenderer& vulkan, ShaderModules& modules);
    ShaderPositionFeedback(const ShaderPositionFeedback& other) = delete;
    ShaderPositionFeedback(ShaderPositionFeedback&& other) = default;
    ShaderPositionFeedback& operator=(const ShaderPositionFeedback& other) = delete;
    ShaderPositionFeedback& operator=(ShaderPositionFeedback&& other) = default;
};
} // namespace Engine
