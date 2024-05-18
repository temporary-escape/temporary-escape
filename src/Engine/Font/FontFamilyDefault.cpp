#include "FontFamilyDefault.hpp"

using namespace Engine;

FontFamilyDefault::FontFamilyDefault(const Config& config, VulkanRenderer& vulkan, const int size) :
    FontFamily{vulkan,
               {
                   config.fontsPath / "IosevkaAile-Regular.ttf",
                   config.fontsPath / "IosevkaAile-Italic.ttf",
                   config.fontsPath / "IosevkaAile-ExtraBold.ttf",
                   config.fontsPath / "IosevkaAile-ExtraBoldItalic.ttf",
                   config.fontsPath / "IosevkaAile-Light.ttf",
                   config.fontsPath / "IosevkaAile-LightItalic.ttf",
               },
               size} {
}
