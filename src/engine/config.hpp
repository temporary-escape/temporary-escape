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

#define GAME_VERSION_STR(V) #V

struct Config {
    std::chrono::microseconds tickLengthUs = 50000us;

    // Window related variables
    std::string windowName = "Temporary Escape";

    struct Graphics {
        int windowWidth = 1920;
        int windowHeight = 1080;
        bool enableValidationLayers = true;
        bool vsync = true;
        size_t maxFramesInFlight = 2;
        float bloomStrength = 0.10f;
        float bloomPower = 1.0f;
        float exposure = 1.0f;
        float gamma = 2.2f;
        float contrast = 1.0f;
        float anisotropy = 4.0f;
        int skyboxIrradianceSize = 32;
        int skyboxPrefilterSize = 128;
        int skyboxSize = 2048;
        int imageAtlasSize = 4096;
        int brdfSize = 512;
        int planetTextureSize = 2048;
        int planetLowResTextureSize = 128;
        bool debugDraw = false;
    } graphics;

    // Paths of interests
    std::filesystem::path assetsPath;
    std::filesystem::path userdataPath;
    std::filesystem::path cwdPath;
    std::filesystem::path fontsPath;
    std::filesystem::path userdataSavesPath;
    std::filesystem::path shapesPath;
    std::filesystem::path pythonHome;

    std::optional<std::string> saveFolderName;
    bool saveFolderClean = false;
    bool voxelTest = false;
    std::string serverPassword = "";
    int serverPort = 22443;
    float cameraFov = 75.0f;
    int thumbnailSize = 256;
    int guiFontSize = 18;
    std::string guiFontName = "iosevka-aile";

    struct Server {
        size_t dbCacheSize{256};
        bool dbDebug{false};
        bool dbCompression{true};
    } server;

    struct Gui {
        float dragAndDropSize{96.0f};
        float actionBarSize{96.0f};
        float sideMenuSize{64.0f};
    } gui;

    struct Input {
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
