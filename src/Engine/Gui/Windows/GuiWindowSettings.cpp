#include "GuiWindowSettings.hpp"
#include "../GuiManager.hpp"

using namespace Engine;

static void addOption(GuiWidgetLayout& layout, const std::string& name, bool& value) {
    auto& row = layout.addWidget<GuiWidgetRow>(25.0f, 2);

    auto& label = row.addWidget<GuiWidgetLabel>(name);
    label.setWidth(0.5f);

    auto& checkbox = row.addWidget<GuiWidgetCheckbox>("", value);
    checkbox.setWidth(0.5f);
}

template <typename T>
static GuiWidgetCombo& addOption(GuiWidgetLayout& layout, const std::string& name, T& value,
                                 const std::vector<std::tuple<std::string, T>>& choices) {

    auto& row = layout.addWidget<GuiWidgetRow>(25.0f, 2);

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

static std::vector<std::tuple<std::string, std::string>> getMonitorNames() {
    std::vector<std::tuple<std::string, std::string>> results;

    for (const auto& info : listSystemMonitors()) {
        results.emplace_back(info.name, info.name);
    }

    return results;
}

GuiWindowSettings::GuiWindowSettings(GuiContext& ctx, const FontFamily& fontFamily, int fontSize,
                                     VulkanRenderer& vulkan, Config& config, GuiManager& guiManager) :
    GuiWindow{ctx, fontFamily, fontSize}, vulkan{vulkan}, config{config}, guiManager{guiManager}, configBackup{config} {

    setSize({450.0f, 500.0f});
    setTitle("SETTINGS");
    setNoScrollbar(true);
    setCloseable(true);

    auto& tabs = addWidget<GuiWidgetTabs>(getSize().y - 75.0f);

    { // General tab
        auto& tab = tabs.addTab("General");

        addOption(tab, "Monitor", config.graphics.monitorName, getMonitorNames());
        addOption(tab,
                  "Window Mode",
                  config.graphics.windowMode,
                  {
                      {"Fullscreen", WindowMode::FullScreen},
                      {"Windowed", WindowMode::Windowed},
                      {"Borderless", WindowMode::Borderless},
                  });
        addOption(tab, "Resolution", config.graphics.windowSize, getVideoModes(vulkan));
        addOption(tab,
                  "UI Scale",
                  config.gui.scale,
                  {
                      {"100%", 1.0f},
                      {"150%", 0.75f},
                      {"200%", 0.5f},
                  });
    }

    { // Graphics tab
        auto& tab = tabs.addTab("Graphics");

        addOption(tab, "V-Sync", config.graphics.vsync);
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

    auto& footer = addWidget<GuiWidgetRow>(30.0f, 3);
    footer.addEmpty().setWidth(0.375f);

    auto& save = footer.addWidget<GuiWidgetButton>("Save");
    save.setStyle(&GuiWidgetButton::successStyle);
    save.setWidth(0.25f);
    save.setOnClick([this]() { onSave(); });

    setOnClose([this]() { onCancel(); });

    footer.addEmpty().setWidth(0.375f);
}

void GuiWindowSettings::onSave() {
    if (onApplyCallback) {
        onApplyCallback();
    }

    auto* modal = this->guiManager.modal(
        "Confirm",
        "Confirm the changes?",
        {"Yes", "No"},
        [this](const std::string& choice) {
            onModalClick(choice);
            return true;
        },
        10);
    modal->setHeaderPrimary(true);
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
