#pragma once

#include "../utils/path.hpp"
#include "vg_types.hpp"

namespace Engine {
class VgShaderModule {
public:
    VgShaderModule() = default;
    explicit VgShaderModule(const Config& config, VkDevice device, const std::string& glsl,
                            VkShaderStageFlagBits stage);
    explicit VgShaderModule(const Config& config, VkDevice device, const Path& path, VkShaderStageFlagBits stage);
    ~VgShaderModule();
    VgShaderModule(const VgShaderModule& other) = delete;
    VgShaderModule(VgShaderModule&& other) noexcept;
    VgShaderModule& operator=(const VgShaderModule& other) = delete;
    VgShaderModule& operator=(VgShaderModule&& other) noexcept;
    void swap(VgShaderModule& other) noexcept;

    VkShaderStageFlagBits getStage() const {
        return stage;
    }

    VkShaderModule& getHandle() {
        return shaderModule;
    }

    const VkShaderModule& getHandle() const {
        return shaderModule;
    }

    operator bool() const {
        return shaderModule != VK_NULL_HANDLE;
    }

    void destroy();

private:
    VkDevice device{VK_NULL_HANDLE};
    VkShaderModule shaderModule{VK_NULL_HANDLE};
    VkShaderStageFlagBits stage;
};
} // namespace Engine
