#pragma once

#include "../Vulkan/VulkanDevice.hpp"

namespace Engine {
class ENGINE_API Registry;

class ENGINE_API Asset {
public:
    explicit Asset(std::string name) : name(std::move(name)) {
    }
    virtual ~Asset() = default;

    virtual void load(Registry& registry, VulkanDevice& vulkan) = 0;

    [[nodiscard]] const std::string& getName() const {
        return name;
    }

private:
    std::string name;
};

using AssetPtr = std::shared_ptr<Asset>;
} // namespace Engine
