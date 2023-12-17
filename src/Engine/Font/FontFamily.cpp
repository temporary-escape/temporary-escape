#include "FontFamily.hpp"

using namespace Engine;

FontFamily::FontFamily(VulkanRenderer& vulkan, const Sources& sources, const int size) :
    faces{
        FontFace{vulkan, sources.regular, size},
        FontFace{vulkan, sources.bold, size},
        FontFace{vulkan, sources.thin, size},
        FontFace{vulkan, sources.light, size},
    } {
}
