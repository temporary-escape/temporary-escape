#pragma once

#include "Utils/Path.hpp"
#include <chrono>
#include <optional>
#include <vector>

namespace Engine {
struct Config {
    std::chrono::microseconds tickLengthUs =
        std::chrono::microseconds(static_cast<int64_t>((1 / 20.0f) * 1000.0 * 1000.0));

    // Window related variables
    std::string windowName = "Temporary Escape";
    int windowWidth = 1920;
    int windowHeight = 1080;

    // Paths of interests
    Path assetsPath;
    Path userdataPath;
    Path cwdPath;
    Path fontsPath;
    Path userdataSavesPath;
    Path shadersPath;
    Path shapesPath;

    std::optional<std::string> saveFolderName;
    bool saveFolderClean = false;
    bool voxelTest = false;
    std::vector<std::string> wrenPaths;
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
        int regionMaxCount{20};
        float factionDistance{20.0f};
        int factionMaxCount{30};
        int systemPlanetsMin{2};
        int systemPlanetsMax{7};
        int planetMoonsMin{0};
        int planetMoonsMax{3};
    } generator;
};
} // namespace Engine
