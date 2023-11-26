#pragma once

#include "../Utils/Path.hpp"
#include "../Vulkan/VulkanShader.hpp"
#include "Asset.hpp"

namespace Engine {
class ENGINE_API Shader : public Asset {
public:
    explicit Shader(std::string name, Path path);
    MOVEABLE(Shader);

    void load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) override;

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
