#pragma once

#include "enums.hpp"
#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

using namespace std::literals::chrono_literals;

namespace Engine {
struct KeyBinding {
    Key key{Key::None};
    Modifier modifier{Modifier::None};

    bool operator()(const Key k, const Modifiers m) const {
        if (modifier != Modifier::None) {
            return k == key && m & static_cast<int>(m);
        } else {
            return k == key;
        }
    }
};

struct Config {
    std::chrono::microseconds tickLengthUs = 50000us;

    // Window related variables
    std::string windowName = "Temporary Escape";
    int windowWidth = 1920;
    int windowHeight = 1080;

    struct {
        bool enableValidationLayers = true;
        bool vsync = true;
        size_t maxFramesInFlight = 2;
    } vulkan;

    // Paths of interests
    std::filesystem::path assetsPath;
    std::filesystem::path userdataPath;
    std::filesystem::path cwdPath;
    std::filesystem::path fontsPath;
    std::filesystem::path userdataSavesPath;
    std::filesystem::path shaderCachePath;
    std::filesystem::path shadersPath;
    std::filesystem::path shapesPath;

    std::optional<std::string> saveFolderName;
    bool saveFolderClean = false;
    bool voxelTest = false;
    std::string serverPassword;
    int serverPort = 22443;
    int skyboxIrradianceSize = 32;
    int skyboxPrefilterSize = 128;
    int skyboxSize = 2048;
    int imageAtlasSize = 4096;
    int brdfSize = 512;
    float cameraFov = 75.0f;
    int thumbnailSize = 128;
    std::string guiFontFaceRegular = "iosevka-aile-regular";
    std::string guiFontFaceBold = "iosevka-aile-bold";
    int guiFontSize = 18;

    struct {
        int totalSystems{2000};
        float galaxyWidth{300.0f};
        float regionDistance{50.0f};
        int regionMaxCount{20};
        float factionDistance{20.0f};
        int factionMaxCount{30};
        int factionSystemsMin{15};
        int factionSystemsMax{75};
        float planetDistanceMin{10.0f};
        float planetDistanceMax{40.0f};
        float planetStartDistance{20.0f};
        float moonDistanceMin{3.0f};
        float moonDistanceMax{6.0f};
        float moonStartDistance{5.0f};
        int systemPlanetsMin{2};
        int systemPlanetsMax{7};
        int planetMoonsMin{0};
        int planetMoonsMax{3};
    } generator;

    struct {
        KeyBinding cameraForward{Key::LetterW};
        KeyBinding cameraBackwards{Key::LetterS};
        KeyBinding cameraLeft{Key::LetterA};
        KeyBinding cameraRight{Key::LetterD};
        KeyBinding cameraUp{Key::SpaceBar};
        KeyBinding cameraDown{Key::LeftControl};
        KeyBinding cameraFast{Key::LeftShift};

        KeyBinding galaxyMapToggle{Key::LetterM};
        KeyBinding systemMapToggle{Key::LetterN};
    } input;
};

static const inline Config defaultConfig{};
} // namespace Engine