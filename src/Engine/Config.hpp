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

enum class WindowMode {
    Windowed,
    FullScreen,
    Borderless,
};

enum class MultiplayerMode {
    Singleplayer,
    LocalLan,
    Online,
};

XML_DEFINE_ENUM(WindowMode, Windowed, FullScreen, Borderless);

struct Config {
    std::chrono::microseconds tickLengthUs = 50000us;

    // Window related variables
    std::string windowName = "Temporary Escape";

    struct Graphics {
        Vector2i windowSize = {1920, 1080};
        WindowMode windowMode = WindowMode::Windowed;
        std::string monitorName;
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
            xml.convert("windowMode", windowMode);
            xml.convert("monitorName", monitorName);
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
            xml.pack("windowMode", windowMode);
            xml.pack("monitorName", monitorName);
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
    std::filesystem::path fontsPath;
    std::filesystem::path userdataPath;
    std::filesystem::path cwdPath;
    std::filesystem::path userdataSavesPath;

    std::optional<std::string> saveFolderName;
    bool saveFolderClean{false};
    bool autoStart{false};
    float cameraFov = 75.0f;
    int thumbnailSize = 256;
    int guiFontSize = 16;

    struct Server {
        uint64_t dbCacheSize{256};
        bool dbDebug{false};
        bool dbCompression{true};

        void convert(const Xml::Node& xml) {
            xml.convert("dbCacheSize", dbCacheSize);
            xml.convert("dbDebug", dbDebug);
            xml.convert("dbCompression", dbCompression);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("dbCacheSize", dbCacheSize);
            xml.pack("dbDebug", dbDebug);
            xml.pack("dbCompression", dbCompression);
        }
    } server;

    struct Gui {
        float scale{1.0f};
        float dragAndDropSize{64.0f};
        float actionBarSize{64.0f};
        float sideMenuSize{64.0f};

        void convert(const Xml::Node& xml) {
            xml.convert("scale", scale);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("scale", scale);
        }
    } gui;

    struct Network {
        std::vector<std::string> stunServers = {
            "stun.l.google.com:19302",
            "stun1.l.google.com:19302",
            "stun2.l.google.com:19302",
            "stun3.l.google.com:19302",
            "stun4.l.google.com:19302",
        };
        std::string clientBindAddress{"0.0.0.0"};
        std::string serverBindAddress{"0.0.0.0"};
        uint16_t serverPort{22443};
        std::string matchmakerUrl{"https://server.temporaryescape.org"};
        uint32_t pkeyLength{2048};

        void convert(const Xml::Node& xml) {
            xml.convert("serverPort", serverPort);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("serverPort", serverPort);
        }
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
        xml.convert("gui", gui);
        xml.convert("server", server);
        xml.convert("network", network);
    }

    void pack(Xml::Node& xml) const {
        xml.pack("graphics", graphics);
        xml.pack("gui", gui);
        xml.pack("server", server);
        xml.pack("network", network);
    }
};

struct Colors {
    static constexpr Color4 primary{hexColor(0xf5b149ff)};
    static constexpr Color4 primaryBackground{hexColor(0x59431bff)};
    static constexpr Color4 success{hexColor(0x5acca4ff)};
    static constexpr Color4 successBackground{hexColor(0x1f3b31ff)};
    static constexpr Color4 danger{hexColor(0xe56865ff)};
    static constexpr Color4 dangerBackground{hexColor(0x471617ff)};
    static constexpr Color4 info{hexColor(0x55e2e5ff)};
    static constexpr Color4 infoBackground{hexColor(0x314141ff)};
    static constexpr Color4 link{hexColor(0xac66e5ff)};
    static constexpr Color4 linkBackground{hexColor(0x311b34ff)};
    static constexpr Color4 background{hexColor(0x323b44ff)};
    static constexpr Color4 border{hexColor(0x455a64ff)};
    static constexpr Color4 text{0.97f, 0.97f, 0.97f, 1.0f};
    static constexpr Color4 textGray{0.6f, 0.6f, 0.6f, 1.0f};
    static constexpr Color4 overlayText{0.97f, 0.97f, 0.97f, 0.7f};
    static constexpr Color4 transparent{0.0f, 0.0f, 0.0f, 0.0f};
    static constexpr Color4 white{hexColor(0xe5e5e3ff)};
    static constexpr Color4 black{hexColor(0x030303ff)};
    static constexpr Color4 tacticalOverview{0.8f, 0.8f, 0.8f, 0.2f};
};

XML_DEFINE(Config, "settings");
} // namespace Engine
