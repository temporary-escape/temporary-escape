#pragma once

#include "font_face.hpp"

namespace Engine {
class ENGINE_API FontFamily {
public:
    explicit FontFamily(VulkanDevice& vulkan, const Path& dir, const std::string& name, float size);

    FontFace regular;
    FontFace bold;
    FontFace thin;
    FontFace light;
};
} // namespace Engine
