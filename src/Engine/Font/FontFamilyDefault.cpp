#include "FontFamilyDefault.hpp"
#include <iosevka-aile-bold-italic.ttf.h>
#include <iosevka-aile-bold.ttf.h>
#include <iosevka-aile-light-italic.ttf.h>
#include <iosevka-aile-light.ttf.h>
#include <iosevka-aile-regular-italic.ttf.h>
#include <iosevka-aile-regular.ttf.h>

using namespace Engine;

static const FontFamily::Sources sources{
    Embed::iosevka_aile_regular_ttf,
    Embed::iosevka_aile_regular_italic_ttf,
    Embed::iosevka_aile_bold_ttf,
    Embed::iosevka_aile_bold_italic_ttf,
    Embed::iosevka_aile_light_ttf,
    Embed::iosevka_aile_light_italic_ttf,
};

FontFamilyDefault::FontFamilyDefault(VulkanRenderer& vulkan, const int size) : FontFamily{vulkan, sources, size} {
}
