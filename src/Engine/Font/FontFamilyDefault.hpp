#pragma once

#include "FontFamily.hpp"

namespace Engine {
class ENGINE_API FontFamilyDefault : public FontFamily {
public:
    FontFamilyDefault(const Config& config, VulkanRenderer& vulkan, int size);
};
} // namespace Engine
