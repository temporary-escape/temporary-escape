#include "gui.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

static Gui::Preferences loadFromYaml(const Config& config) {
    Gui::Preferences preferences{};
    if (Fs::exists(config.userdataPath / "gui.yaml")) {
        try {
            preferences.fromYaml(config.userdataPath / "gui.yaml");
        } catch (std::exception& e) {
            BACKTRACE(e, "Failed to load GUI preferences");
        }
    }
    return preferences;
}

Gui::Gui(const Config& config, Registry& registry, VoxelPalette& voxelPalette) :
    config{config},
    preferences{loadFromYaml(config)},
    blockActionBar{config, preferences.blockActionBar, registry, voxelPalette} {

    contextMenu.setEnabled(false);
    blockSelector.setEnabled(false);
    blockActionBar.setEnabled(false);
}

Gui::~Gui() noexcept {
    try {
        preferences.toYaml(config.userdataPath / "gui.yaml");
    } catch (std::exception& e) {
        BACKTRACE(e, "Failed to save GUI preferences");
    }
}

void Gui::draw(Nuklear& nuklear, const Vector2& viewport) {
    contextMenu.draw(nuklear, viewport);
    blockSelector.draw(nuklear, viewport);
    blockActionBar.draw(nuklear, viewport);
}
