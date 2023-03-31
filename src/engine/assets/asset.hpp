#pragma once

#include "../vulkan/vulkan_renderer.hpp"
#include <msgpack.hpp>

namespace Engine {
class ENGINE_API Registry;

class ENGINE_API Asset : public NonCopyable {
public:
    explicit Asset(std::string name) : name(std::move(name)) {
    }
    virtual ~Asset() = default;
    MOVEABLE(Asset);

    virtual void load(Registry& registry, VulkanRenderer& vulkan) = 0;

    [[nodiscard]] const std::string& getName() const {
        return name;
    }

private:
    std::string name;
};

using AssetPtr = std::shared_ptr<Asset>;
} // namespace Engine
