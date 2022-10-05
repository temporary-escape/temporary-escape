#pragma once

#include "../Library.hpp"
#include "../Utils/Path.hpp"
#include "VEZ/VEZ.h"
#include <string>
#include <type_traits>
#include <vector>

namespace Engine {
enum class ShaderType {
    Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
    Vertex = VK_SHADER_STAGE_VERTEX_BIT,
    Geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
};

struct ShaderSource {
    Path path;
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

    [[nodiscard]] operator bool() const {
        return desc.pipeline != VK_NULL_HANDLE;
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
