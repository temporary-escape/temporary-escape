#pragma once

#include "../Vulkan/VulkanTexture.hpp"

namespace Engine {
class ENGINE_API VulkanRenderer;

class ENGINE_API BannerImage : public VulkanDisposable {
public:
    BannerImage(VulkanRenderer& vulkan);

    void destroy() override;

    const VulkanTexture& get() const {
        return texture;
    }

private:
    VulkanTexture texture;
};
} // namespace Engine
