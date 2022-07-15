#pragma once

#include "../Library.hpp"
#include "VEZ/VEZ.h"
#include <string>
#include <type_traits>
#include <vector>

namespace Engine {
enum class ShaderType {
    Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
    Vertex = VK_SHADER_STAGE_VERTEX_BIT,
};

struct ShaderSource {
    std::string glsl;
    ShaderType type;
};

class VulkanPipeline {
public:
    NON_COPYABLE(VulkanPipeline);

    VulkanPipeline() = default;
    explicit VulkanPipeline(VkDevice device, VezPipeline pipeline, std::vector<VkShaderModule> shaderModules);
    ~VulkanPipeline();
    VulkanPipeline(VulkanPipeline&& other) noexcept;
    VulkanPipeline& operator=(VulkanPipeline&& other) noexcept;
    void swap(VulkanPipeline& other) noexcept;
    void reset();

    [[nodiscard]] VezPipeline& getHandle() {
        return desc.pipeline;
    }

    [[nodiscard]] const VezPipeline& getHandle() const {
        return desc.pipeline;
    }

private:
    struct PipelineDescription {
        VezPipeline pipeline = VK_NULL_HANDLE;
        std::vector<VkShaderModule> shaderModules;
    };

    VkDevice device{VK_NULL_HANDLE};
    PipelineDescription desc{};
};

static_assert(std::is_move_constructible<VulkanPipeline>::value, "VulkanPipeline must be move constructible");
static_assert(std::is_move_assignable<VulkanPipeline>::value, "VulkanPipeline must be move assignable");
} // namespace Engine
