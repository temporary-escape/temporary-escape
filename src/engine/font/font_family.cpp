#include "font_family.hpp"

using namespace Engine;

FontFamily::FontFamily(VulkanRenderer& vulkan, const Path& dir, const std::string& name, const float size) :
    regular{vulkan, dir / Path{name + "-regular.ttf"}, size},
    bold{vulkan, dir / Path{name + "-bold.ttf"}, size},
    thin{vulkan, dir / Path{name + "-thin.ttf"}, size},
    light{vulkan, dir / Path{name + "-light.ttf"}, size} {
}
