#pragma once

#include "Utils/Path.hpp"

namespace Scissio {
struct Config {
    Path resourcesPath;
    Path assetsPath;
    Path userdataPath;
    Path cwdPath;
    Path userdataSavesPath;
    Path shadersPath;
    int serverPort = 22443;
    int skyboxIrradianceSize = 32;
    int skyboxPrefilterSize = 128;
    int skyboxSize = 2048;
    int imageAtlasSize = 4096;
    int brdfSize = 512;
    int fboSamples = 4;
    float cameraFov = 75.0f;
    int thumbnailSize = 128;
    std::string guiFontFaceRegular = "iosevka-aile-regular";
    std::string guiFontFaceBold = "iosevka-aile-bold";
    int guiFontSize = 21;
};
} // namespace Scissio
