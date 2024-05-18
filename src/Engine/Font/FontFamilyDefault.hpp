#pragma once

#include "FontFamily.hpp"

namespace Engine {
class ENGINE_API FontFamilyDefault : public FontFamily {
public:
    FontFamilyDefault(VulkanRenderer& vulkan, int size);
};
}
