#include "FontFamilyDefault.hpp"
#include <iosevka-aile-bold.ttf.h>
#include <iosevka-aile-light.ttf.h>
#include <iosevka-aile-regular.ttf.h>

using namespace Engine;

const ENGINE_API FontFamily::Sources Engine::defaultFontSources{
    Embed::iosevka_aile_regular_ttf,
    Embed::iosevka_aile_bold_ttf,
    Embed::iosevka_aile_light_ttf,
};
