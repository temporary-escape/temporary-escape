#pragma once

#include "../audio/audio_context.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include <msgpack.hpp>

namespace Engine {
class ENGINE_API AssetsManager;

class ENGINE_API Asset : public NonCopyable {
public:
    explicit Asset(std::string name) : name(std::move(name)) {
    }
    virtual ~Asset() = default;
    MOVEABLE(Asset);

    virtual void load(AssetsManager& assetsManager, VulkanRenderer& vulkan, AudioContext& audio) = 0;

    [[nodiscard]] const std::string& getName() const {
        return name;
    }

private:
    std::string name;
};

using AssetPtr = std::shared_ptr<Asset>;
} // namespace Engine
