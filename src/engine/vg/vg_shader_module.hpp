#pragma once

#include "../utils/path.hpp"
#include "vg_types.hpp"

namespace Engine {
class VgRenderer;

class VgShaderModule {
public:
    VgShaderModule() = default;
    explicit VgShaderModule(const Config& config, VgRenderer& renderer, const std::string& glsl,
                            VkShaderStageFlagBits stage);
    explicit VgShaderModule(const Config& config, VgRenderer& renderer, const Path& path, VkShaderStageFlagBits stage);
    ~VgShaderModule();
    VgShaderModule(const VgShaderModule& other) = delete;
    VgShaderModule(VgShaderModule&& other) noexcept;
    VgShaderModule& operator=(const VgShaderModule& other) = delete;
    VgShaderModule& operator=(VgShaderModule&& other) noexcept;
    void swap(VgShaderModule& other) noexcept;

    VkShaderStageFlagBits getStage() const {
        return state->stage;
    }

    VkShaderModule& getHandle() {
        return state->shaderModule;
    }

    const VkShaderModule& getHandle() const {
        return state->shaderModule;
    }

    operator bool() const {
        return state->shaderModule != VK_NULL_HANDLE;
    }

    void destroy();

private:
    class ShaderModuleState : public VgDisposable {
    public:
        void destroy() override;

        VgRenderer* renderer{nullptr};
        VkShaderModule shaderModule{VK_NULL_HANDLE};
        VkShaderStageFlagBits stage;
    };

    std::shared_ptr<ShaderModuleState> state;
};
} // namespace Engine
