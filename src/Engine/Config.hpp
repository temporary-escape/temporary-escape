#pragma once

#include "Enums.hpp"
#include "Utils/Xml.hpp"
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
        Vector2i windowSize = {1920, 1080};
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
        int shadowsLevel = 1;
        Vector2i textureArraySize{1024, 1024};

        void convert(const Xml::Node& xml) {
            xml.convert("windowSize", windowSize);
            xml.convert("fullscreen", fullscreen);
            xml.convert("vsync", vsync);
            xml.convert("anisotropy", anisotropy);
            xml.convert("skyboxSize", skyboxSize);
            xml.convert("planetTextureSize", planetTextureSize);
            xml.convert("fxaa", fxaa);
            xml.convert("bloom", bloom);
            xml.convert("ssao", ssao);
            xml.convert("shadowsSize", shadowsSize);
            xml.convert("shadowsLevel", shadowsLevel);
            xml.convert("debugDraw", debugDraw);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("windowSize", windowSize);
            xml.pack("fullscreen", fullscreen);
            xml.pack("vsync", vsync);
            xml.pack("anisotropy", anisotropy);
            xml.pack("skyboxSize", skyboxSize);
            xml.pack("planetTextureSize", planetTextureSize);
            xml.pack("fxaa", fxaa);
            xml.pack("bloom", bloom);
            xml.pack("ssao", ssao);
            xml.pack("shadowsSize", shadowsSize);
            xml.pack("shadowsLevel", shadowsLevel);
            xml.pack("debugDraw", debugDraw);
        }
    } graphics;

    // Paths of interests
    std::filesystem::path assetsPath;
    std::filesystem::path userdataPath;
    std::filesystem::path cwdPath;
    std::filesystem::path userdataSavesPath;

    std::optional<std::string> saveFolderName;
    bool saveFolderClean{false};
    bool autoStart{false};
    std::string serverPassword;
    int serverPort = 22443;
    float cameraFov = 75.0f;
    int thumbnailSize = 256;
    int guiFontSize = 18;

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

    struct Network {
        std::vector<std::string> stunServers = {
            "stun.l.google.com:19302",
            "stun1.l.google.com:19302",
            "stun2.l.google.com:19302",
            "stun3.l.google.com:19302",
            "stun4.l.google.com:19302",
        };
        std::string serverBindAddress{"0.0.0.0"};
    } network;

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

struct Colors {
    static constexpr Color4 primary{0.99f, 0.86f, 0.05f, 1.0f};
    static constexpr Color4 secondary{0.024f, 0.81f, 0.69f, 1.0f};
    static constexpr Color4 ternary{0.91f, 0.0f, 0.05f, 1.0f};
    static constexpr Color4 background{0.02f, 0.02f, 0.02f, 1.0f};
    static constexpr Color4 border{hexColor(0x202020ff)};
    static constexpr Color4 text{0.9f, 0.9f, 0.9f, 1.0f};
    static constexpr Color4 overlayText{0.9f, 0.9f, 0.9f, 0.7f};
    static constexpr Color4 transparent{0.0f, 0.0f, 0.0f, 0.0f};
};

XML_DEFINE(Config, "settings");
} // namespace Engine
