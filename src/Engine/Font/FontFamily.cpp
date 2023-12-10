#include "FontFamily.hpp"

using namespace Engine;

FontFamily::FontFamily(VulkanRenderer& vulkan, const Path& dir, const std::string& name, const int size) :
    faces{
        FontFace{vulkan, dir / Path{name + "-regular.ttf"}, size},
        FontFace{vulkan, dir / Path{name + "-bold.ttf"}, size},
        FontFace{vulkan, dir / Path{name + "-thin.ttf"}, size},
        FontFace{vulkan, dir / Path{name + "-light.ttf"}, size},
    } {
}
