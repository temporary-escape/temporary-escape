#pragma once

#include "VulkanTypes.hpp"

namespace spirv_cross {
class CompilerGLSL;
}

namespace Engine {
class ENGINE_API SpirvReflection : public NonCopyable {
public:
    explicit SpirvReflection(const std::vector<uint32_t>& spriv);
    virtual ~SpirvReflection();
    SpirvReflection(SpirvReflection&& other) noexcept;
    SpirvReflection& operator=(SpirvReflection&& other) noexcept;

    const std::vector<VulkanStageInput>& getInputs() const {
        return inputs;
    }

    const std::vector<VulkanStageUniform>& getUniforms() const {
        return uniforms;
    }

    const std::vector<VulkanStageSampler>& getSamplers() const {
        return samplers;
    }

    const std::vector<VulkanStageSampler>& getSubpassInputs() const {
        return subpassInputs;
    }

    const std::vector<VulkanStageStorageBuffer>& getStorageBuffers() const {
        return storageBuffers;
    }

    const VulkanStagePushConstants& getPushConstants() const {
        return pushConstants;
    }

private:
    std::unique_ptr<spirv_cross::CompilerGLSL> compiler;
    std::vector<VulkanStageInput> inputs;
    std::vector<VulkanStageUniform> uniforms;
    std::vector<VulkanStageStorageBuffer> storageBuffers;
    std::vector<VulkanStageSampler> samplers;
    std::vector<VulkanStageSampler> subpassInputs;
    VulkanStagePushConstants pushConstants{};
};
} // namespace Engine
