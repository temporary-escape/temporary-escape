#pragma once

#include "../utils/path.hpp"
#include "../vulkan/vulkan_shader.hpp"
#include "asset.hpp"

namespace Engine {
class ENGINE_API Shader : public Asset {
public:
    explicit Shader(std::string name, Path path);
    MOVEABLE(Shader);

    void load(Registry& registry, VulkanRenderer& vulkan) override;

    static std::shared_ptr<Shader> from(const std::string& name);

    const VulkanShader& getVulkanShader() const {
        return shader;
    }

    VulkanShader& getVulkanShader() {
        return shader;
    }

    VkShaderStageFlagBits getStage() const {
        return stage;
    }

private:
    Path path;
    VulkanShader shader;
    VkShaderStageFlagBits stage;
};

using ShaderPtr = std::shared_ptr<Shader>;
} // namespace Engine
