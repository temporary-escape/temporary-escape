#include "gui_keep_settings.hpp"

using namespace Engine;

static const std::array<std::string, 4> tabsTitle = {
    "Game",
    "Video",
    "Audio",
    "Input",
};

GuiKeepSettings::GuiKeepSettings() {
    setSize({300.0f, 150.0f});
    setFlags(Nuklear::WindowFlags::Border | Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Title);
    setTitle("Keep Settings?");
}

void GuiKeepSettings::drawLayout(Nuklear& nuklear) {
    nuklear.layoutDynamic(0.0f, 1);
    nuklear.label("Keep the current settings?");

    nuklear.layoutDynamic(0.0f, 1);
    nuklear.progress(progress);

    nuklear.layoutDynamic(0.0f, 4);
    nuklear.layoutSkip();
    if (nuklear.button("No")) {
        onResult(false);
        onResult = nullptr;
        setEnabled(false);
    }
    if (nuklear.button("Yes")) {
        onResult(true);
        onResult = nullptr;
        setEnabled(false);
    }
    nuklear.layoutSkip();
}

void GuiKeepSettings::beforeDraw(Nuklear& nuklear, const Vector2& viewport) {
    setPos({viewport.x / 2 - getSize().x / 2, viewport.y / 2 - getSize().y / 2});
}

void GuiKeepSettings::reset() {
    progress = 1.0f;
}

void GuiKeepSettings::updateProgress(const float deltaTime) {
    progress -= deltaTime * (1.0f / timeout);
    if (progress < 0.0f) {
        progress = 0.0f;

        if (onResult) {
            onResult(false);
            onResult = nullptr;
            setEnabled(false);
        }
    }
}
