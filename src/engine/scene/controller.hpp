#pragma once

#include "../vulkan/vulkan_renderer.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API Scene;

class ENGINE_API Controller {
public:
    virtual ~Controller() = default;

    virtual void update(float delta) = 0;
    virtual void recalculate(VulkanRenderer& vulkan) = 0;
};
} // namespace Engine
