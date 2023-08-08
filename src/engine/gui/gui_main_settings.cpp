#include "gui_main_settings.hpp"

using namespace Engine;

static const std::array<std::string, 4> tabsTitle = {
    "Game",
    "Video",
    "Audio",
    "Input",
};

GuiMainSettings::GuiMainSettings() {
    setSize({500.0f, 500.0f});
    setFlags(Nuklear::WindowFlags::Border | Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Title);
    setTitle("Settings");
}

void GuiMainSettings::setVideoModes(std::vector<Vector2i> value) {
    videoModes = std::move(value);
    videoModesStr.reserve(videoModes.size());
    for (size_t i = 0; i < videoModes.size(); i++) {
        auto& videoMode = videoModes.at(i);
        videoModesStr.push_back(fmt::format("{}x{}", videoMode.x, videoMode.y));
        if (config.graphics.windowWidth == videoMode.x && config.graphics.windowHeight == videoMode.y) {
            videoModeChosen = i;
        }
    }
}

void GuiMainSettings::drawLayout(Nuklear& nuklear) {
    drawLayoutTabs(nuklear);

    const auto windowBounds = nuklear.getContentRegion() - Vector2{0.0f, 70.0f};
    nuklear.layoutStatic(windowBounds.y, windowBounds.x, 1);

    nuklear.groupBegin("Settings Group", true);
    if (tabsValue == 1) {
        drawLayoutGraphics(nuklear);
    }
    nuklear.groupEnd();

    drawBottomBar(nuklear);
}

void GuiMainSettings::drawLayoutTabs(Nuklear& nuklear) {
    nuklear.layoutDynamic(30.0f, 4);
    for (size_t i = 0; i < tabsTitle.size(); i++) {
        auto active = tabsValue == i;
        nuklear.buttonToggle(tabsTitle.at(i), active);
        if (active) {
            tabsValue = i;
        }
    }
}

static void drawEmpty(Nuklear& nuklear) {
    nuklear.layoutDynamic(0.0f, 1);
    nuklear.layoutSkip();
}

static void drawSeparator(Nuklear& nuklear, const std::string& name) {
    nuklear.layoutDynamic(0.0f, 1);
    nuklear.label(name);
}

static void drawProperty(Nuklear& nuklear, const std::string& name) {
    nuklear.layoutDynamic(0.0f, 2);
    nuklear.label(name);
}

static void drawPropertyToggle(Nuklear& nuklear, const std::string& name, bool& value) {
    static const std::vector<std::string> items = {
        "Disabled",
        "Enabled",
    };

    drawProperty(nuklear, name);

    size_t choice = value ? 1 : 0;
    Vector2 size{
        nuklear.getWidgetSize().x,
        200.0f,
    };
    nuklear.combo(size, choice, items);
    value = choice == 1;
}

static void drawPropertyChoices(Nuklear& nuklear, const std::string& name, size_t& choice,
                                const std::vector<std::string>& items) {
    drawProperty(nuklear, name);

    Vector2 size{
        nuklear.getWidgetSize().x,
        200.0f,
    };
    nuklear.combo(size, choice, items);
}

static void drawPropertyChoices(Nuklear& nuklear, const std::string& name, int& value, const std::vector<int>& values,
                                const std::vector<std::string>& items) {
    drawProperty(nuklear, name);

    Vector2 size{
        nuklear.getWidgetSize().x,
        200.0f,
    };
    size_t choice = 0;
    for (size_t i = 0; i < items.size(); i++) {
        if (value == values.at(i)) {
            choice = i;
        }
    }
    nuklear.combo(size, choice, items);
    value = values.at(choice);
}

void GuiMainSettings::drawLayoutGraphics(Nuklear& nuklear) {
    static const std::vector<std::string> lowMediumHighChoices = {
        "Low",
        "Medium",
        "High",
    };
    static const std::vector<std::string> offLowHighChoices = {
        "Low",
        "Medium",
        "High",
    };
    static const std::vector<std::string> offLowMediumHighChoices = {
        "Off",
        "Low",
        "Medium",
        "High",
    };

    drawSeparator(nuklear, "Main video settings:");
    drawPropertyChoices(nuklear, "  Resolution", videoModeChosen, videoModesStr);
    drawPropertyToggle(nuklear, "  Fullscreen", config.graphics.fullscreen);
    drawPropertyToggle(nuklear, "  VSync", config.graphics.vsync);
    drawEmpty(nuklear);

    drawSeparator(nuklear, "Advanced settings:");

    drawPropertyToggle(nuklear, "  Anti-Alias", config.graphics.fxaa);
    drawPropertyToggle(nuklear, "  Bloom", config.graphics.bloom);

    static const std::vector<int> anisotropicValues = {
        1,
        4,
        8,
        16,
    };
    drawPropertyChoices(
        nuklear, "  Texture filtering", config.graphics.anisotropy, anisotropicValues, offLowMediumHighChoices);

    static const std::vector<int> skyboxValues = {
        1024,
        2048,
        4096,
    };
    drawPropertyChoices(nuklear, "  Skybox", config.graphics.skyboxSize, skyboxValues, lowMediumHighChoices);

    static const std::vector<int> planetsValues = {
        1024,
        2048,
        4096,
    };
    drawPropertyChoices(nuklear, "  Planets", config.graphics.planetTextureSize, planetsValues, lowMediumHighChoices);

    static const std::vector<int> ssaoValues = {
        0,
        16,
        32,
        64,
    };
    drawPropertyChoices(nuklear, "  SSAO", config.graphics.ssao, ssaoValues, offLowMediumHighChoices);

    static const std::vector<int> shadowsValues = {
        0,
        1024,
        2048,
        4096,
    };
    drawPropertyChoices(nuklear, "  Shadows", config.graphics.shadowsSize, shadowsValues, offLowMediumHighChoices);
}

void GuiMainSettings::drawBottomBar(Nuklear& nuklear) {
    nuklear.layoutDynamic(30.0f, 4);
    nuklear.layoutSkip();
    if (nuklear.button("Cancel")) {
        setEnabled(false);
    }
    if (nuklear.button("Save")) {
        config.graphics.windowWidth = videoModes.at(videoModeChosen).x;
        config.graphics.windowHeight = videoModes.at(videoModeChosen).y;
        onApply(config);
        setEnabled(false);
    }
    nuklear.layoutSkip();
}

void GuiMainSettings::beforeDraw(Nuklear& nuklear, const Vector2& viewport) {
    setPos({viewport.x / 2 - getSize().x / 2, viewport.y / 2 - getSize().y / 2});
}
