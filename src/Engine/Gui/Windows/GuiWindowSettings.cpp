#include "GuiWindowSettings.hpp"
#include "../GuiManager.hpp"

using namespace Engine;

static void addOption(GuiWidgetLayout& layout, const std::string& name, bool& value) {
    auto& row = layout.addWidget<GuiWidgetRow>(30.0f, 2);

    auto& label = row.addWidget<GuiWidgetLabel>(name);
    label.setWidth(0.5f);

    auto& checkbox = row.addWidget<GuiWidgetCheckbox>("", value);
    checkbox.setWidth(0.5f);
}

template <typename T>
static GuiWidgetCombo& addOption(GuiWidgetLayout& layout, const std::string& name, T& value,
                                 const std::vector<std::tuple<std::string, T>>& choices) {

    auto& row = layout.addWidget<GuiWidgetRow>(30.0f, 2);

    auto& label = row.addWidget<GuiWidgetLabel>(name);
    label.setWidth(0.5f);

    auto& combo = row.addWidget<GuiWidgetComboTyped<T>>(value);

    for (const auto& [text, choice] : choices) {
        combo.addChoice(text, choice);
    }

    return combo;
}

static std::vector<std::tuple<std::string, Vector2i>> getVideoModes(VulkanRenderer& vulkan) {
    auto videoModes = vulkan.getSupportedResolutionModes();
    std::sort(videoModes.begin(), videoModes.end(), [](const Vector2i& a, const Vector2i& b) { return a.x > b.x; });

    decltype(videoModes) videoModesFiltered;
    std::copy_if(videoModes.begin(), videoModes.end(), std::back_inserter(videoModesFiltered), [](const Vector2i& a) {
        return a.x >= 1360 && a.y >= 800;
    });

    std::vector<std::tuple<std::string, Vector2i>> result;

    result.reserve(videoModesFiltered.size());
    for (auto videoMode : videoModesFiltered) {
        result.emplace_back(fmt::format("{}x{}", videoMode.x, videoMode.y), videoMode);
    }

    return result;
}

GuiWindowSettings::GuiWindowSettings(const FontFamily& fontFamily, int fontSize, VulkanRenderer& vulkan, Config& config,
                                     GuiManager& guiManager) :
    GuiWindow{fontFamily, fontSize}, vulkan{vulkan}, config{config}, guiManager{guiManager}, configBackup{config} {

    setSize({450.0f, 700.0f});
    setTitle("SETTINGS");
    setNoScrollbar(true);

    auto& tabs = addWidget<GuiWidgetTabs>(600.0f);

    tabs.addTab("General");

    { // Graphics tab
        auto& tab = tabs.addTab("Graphics");

        addOption(tab, "Fullscreen", config.graphics.fullscreen);
        addOption(tab, "V-Sync", config.graphics.vsync);

        addOption(tab, "Resolution", config.graphics.windowSize, getVideoModes(vulkan));

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
    save.setOnClick([this]() { onSave(); });

    auto& cancel = footer.addWidget<GuiWidgetButton>("Cancel");
    cancel.setWidth(0.25f);
    cancel.setOnClick([this]() { onCancel(); });

    footer.addEmpty().setWidth(0.25f);
}

void GuiWindowSettings::onSave() {
    if (onApplyCallback) {
        onApplyCallback();
    }

    this->guiManager.modal(
        "Confirm",
        "Confirm the changes?",
        {"Yes", "No"},
        [this](const std::string& choice) { onModalClick(choice); },
        15);
}

void GuiWindowSettings::onCancel() {
    this->config = configBackup;
    if (onSubmitCallback) {
        onSubmitCallback(false);
    }
}

void GuiWindowSettings::onModalClick(const std::string& choice) {
    if (choice == "Yes") {
        if (onSubmitCallback) {
            onSubmitCallback(true);
        }
    } else {
        this->config = configBackup;

        if (onApplyCallback) {
            onApplyCallback();
        }

        if (onSubmitCallback) {
            onSubmitCallback(false);
        }
    }
}

void GuiWindowSettings::setOnApply(OnApplyCallback callback) {
    onApplyCallback = std::move(callback);
}

void GuiWindowSettings::setOnSubmit(OnSubmitCallback callback) {
    onSubmitCallback = std::move(callback);
}

void GuiWindowSettings::reset() {
    configBackup = config;
    /*size_t currentVideoMode{0};
    getVideoModes(vulkan, config, currentVideoMode);
    comboResolution->setChosen(currentVideoMode);*/
}
