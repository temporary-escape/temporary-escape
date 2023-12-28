#include "GuiWindowSettings.hpp"

using namespace Engine;

static void addOption(GuiWidgetLayout& layout, const std::string& name, bool& value) {
    auto& row = layout.addWidget<GuiWidgetRow>(30.0f, 2);

    auto& label = row.addWidget<GuiWidgetLabel>(name);
    label.setWidth(0.5f);

    auto& checkbox = row.addWidget<GuiWidgetCheckbox>("");
    checkbox.setValue(value);
    checkbox.setWidth(0.5f);
    checkbox.setOnClick([&value](bool toggle) { value = toggle; });
}

static GuiWidgetCombo& addOption(GuiWidgetLayout& layout, const std::string& name,
                                 const std::vector<std::string>& choices) {

    auto& row = layout.addWidget<GuiWidgetRow>(30.0f, 2);

    auto& label = row.addWidget<GuiWidgetLabel>(name);
    label.setWidth(0.5f);

    auto& combo = row.addWidget<GuiWidgetCombo>();
    for (const auto& choice : choices) {
        combo.addChoice(choice);
    }

    return combo;
}

template <typename T>
static GuiWidgetCombo& addOption(GuiWidgetLayout& layout, const std::string& name, T& value,
                                 const std::vector<std::tuple<std::string, T>>& choices) {

    auto& row = layout.addWidget<GuiWidgetRow>(30.0f, 2);

    auto& label = row.addWidget<GuiWidgetLabel>(name);
    label.setWidth(0.5f);

    auto& combo = row.addWidget<GuiWidgetComboTyped<T>>();

    for (const auto& [text, choice] : choices) {
        combo.addChoice(text, choice);
    }
    combo.setValue(value);

    return combo;
}

static std::vector<std::tuple<std::string, Vector2i>> getVideoModes(VulkanRenderer& vulkan) {
    const auto videoModes = vulkan.getSupportedResolutionModes();

    std::vector<std::tuple<std::string, Vector2i>> result;

    result.reserve(videoModes.size());
    for (auto videoMode : videoModes) {
        result.emplace_back(fmt::format("{}x{}", videoMode.x, videoMode.y), videoMode);
    }

    return result;
}

GuiWindowSettings::GuiWindowSettings(const FontFamily& fontFamily, int fontSize, VulkanRenderer& vulkan,
                                     Config& config) :
    GuiWindow2{fontFamily, fontSize}, vulkan{vulkan}, config{config} {

    setSize({450.0f, 700.0f});
    setTitle("Settings");
    setNoScrollbar(true);

    auto& tabs = addWidget<GuiWidgetTabs>(600.0f);

    tabs.addTab("General");

    { // Graphics tab
        auto& tab = tabs.addTab("Graphics");

        addOption(tab, "Fullscreen", config.graphics.fullscreen);
        addOption(tab, "V-Sync", config.graphics.vsync);

        /*size_t currentVideoMode{0};
        comboResolution = &addOption(tab, "Resolution", getVideoModes(vulkan, config, currentVideoMode));
        comboResolution->setChosen(currentVideoMode);*/

        // addOption(tab, "Resolution", config.graphics.windowSize, getVideoModes(vulkan));

        addOption(tab, "Bloom", config.graphics.bloom);
        addOption(tab, "Anti-Aliasing", config.graphics.fxaa);

        addOption(tab,
                  "Texture filtering",
                  config.graphics.anisotropy,
                  {
                      {"Off", 1},
                      {"Low", 4},
                      {"Medium", 8},
                      {"High", 16},
                  });

        addOption(tab,
                  "Background",
                  config.graphics.skyboxSize,
                  {
                      {"Low", 1024},
                      {"Medium", 2048},
                      {"High", 4096},
                  });

        addOption(tab,
                  "Planets",
                  config.graphics.planetTextureSize,
                  {
                      {"Low", 512},
                      {"Medium", 1024},
                      {"High", 2048},
                  });

        addOption(tab,
                  "SSAO",
                  config.graphics.ssao,
                  {
                      {"Off", 0},
                      {"Low", 16},
                      {"Medium", 32},
                      {"High", 64},
                  });

        addOption(tab,
                  "Shadows",
                  config.graphics.shadowsSize,
                  {
                      {"Off", 0},
                      {"Low", 1024},
                      {"Medium", 2048},
                      {"High", 4096},
                  });

        addOption(tab, "Debug Draw", config.graphics.debugDraw);
    }

    tabs.addTab("Audio");
    tabs.addTab("Keyboard");

    auto& footer = addWidget<GuiWidgetRow>(30.0f, 4);
    footer.addEmpty().setWidth(0.25f);

    auto& save = footer.addWidget<GuiWidgetButton>("Save");
    save.setWidth(0.25f);
    save.setOnClick([this]() {
        if (onSubmitCallback) {
            onSubmitCallback(true);
        }
    });

    auto& cancel = footer.addWidget<GuiWidgetButton>("Cancel");
    cancel.setWidth(0.25f);
    cancel.setOnClick([this]() {
        if (onSubmitCallback) {
            onSubmitCallback(false);
        }
    });

    footer.addEmpty().setWidth(0.25f);
}

void GuiWindowSettings::setOnSubmit(GuiWindowSettings::OnSubmitCallback callback) {
    onSubmitCallback = std::move(callback);
}

void GuiWindowSettings::reset() {
    /*size_t currentVideoMode{0};
    getVideoModes(vulkan, config, currentVideoMode);
    comboResolution->setChosen(currentVideoMode);*/
}
