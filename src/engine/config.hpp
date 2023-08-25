#pragma once

#include "enums.hpp"
#include "utils/xml.hpp"
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
        bool fullscreen = false;
        bool enableValidationLayers = true;
        bool vsync = true;
        float bloomStrength = 0.10f;
        float bloomPower = 1.0f;
        float exposure = 1.0f;
        float gamma = 2.2f;
        float contrast = 1.0f;
        int anisotropy = 4;
        int skyboxIrradianceSize = 32;
        int skyboxPrefilterSize = 128;
        int skyboxSize = 2048;
        int imageAtlasSize = 4096;
        int brdfSize = 512;
        int planetTextureSize = 2048;
        int planetLowResTextureSize = 128;
        bool fxaa = true;
        bool bloom = true;
        bool debugDraw = false;
        int ssao = 32;
        int shadowsSize = 2048;

        void convert(const Xml::Node& xml) {
            xml.convert("windowWidth", windowWidth);
            xml.convert("windowHeight", windowHeight);
            xml.convert("fullscreen", fullscreen);
            xml.convert("vsync", vsync);
            xml.convert("anisotropy", anisotropy);
            xml.convert("skyboxSize", skyboxSize);
            xml.convert("planetTextureSize", planetTextureSize);
            xml.convert("fxaa", fxaa);
            xml.convert("bloom", bloom);
            xml.convert("ssao", ssao);
            xml.convert("shadowsSize", shadowsSize);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("windowWidth", windowWidth);
            xml.pack("windowHeight", windowHeight);
            xml.pack("fullscreen", fullscreen);
            xml.pack("vsync", vsync);
            xml.pack("anisotropy", anisotropy);
            xml.pack("skyboxSize", skyboxSize);
            xml.pack("planetTextureSize", planetTextureSize);
            xml.pack("fxaa", fxaa);
            xml.pack("bloom", bloom);
            xml.pack("ssao", ssao);
            xml.pack("shadowsSize", shadowsSize);
        }
    } graphics;

    // Paths of interests
    std::filesystem::path assetsPath;
    std::filesystem::path userdataPath;
    std::filesystem::path cwdPath;
    std::filesystem::path fontsPath;
    std::filesystem::path userdataSavesPath;
    std::filesystem::path shapesPath;

    std::optional<std::string> saveFolderName;
    bool saveFolderClean = false;
    std::string serverPassword;
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
        float dragAndDropSize{64.0f};
        float actionBarSize{64.0f};
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

    void convert(const Xml::Node& xml) {
        xml.convert("graphics", graphics);
    }

    void pack(Xml::Node& xml) const {
        xml.pack("graphics", graphics);
    }
};

XML_DEFINE(Config, "settings");
} // namespace Engine
