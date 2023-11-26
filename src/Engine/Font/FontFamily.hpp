#pragma once

#include "FontFace.hpp"

namespace Engine {
class ENGINE_API FontFamily {
public:
    explicit FontFamily(VulkanRenderer& vulkan, const Path& dir, const std::string& name, float size);

    FontFace regular;
    FontFace bold;
    FontFace thin;
    FontFace light;
};
} // namespace Engine
