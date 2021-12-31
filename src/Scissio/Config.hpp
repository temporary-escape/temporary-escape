#pragma once

#include "Utils/Path.hpp"

namespace Scissio {
struct Config {
    int windowWidth = 1920;
    int windowHeight = 1080;
    Path assetsPath;
    Path userdataPath;
    Path cwdPath;
    Path userdataSavesPath;
    Path shadersPath;
    std::string serverPassword;
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
    int guiFontSize = 18;

    struct GeneratorOptions {
        int totalSystems{2000};
        float galaxyWidth{300.0f};
        float regionDistance{50.0f};
        int systemPlanetsMin{2};
        int systemPlanetsMax{7};
        int planetMoonsMin{0};
        int planetMoonsMax{3};
    } generator;
};
} // namespace Scissio
