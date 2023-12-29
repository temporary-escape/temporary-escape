#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API AssetsManager;

class ENGINE_API GuiWindowShipToolbar : public GuiWindow {
public:
    explicit GuiWindowShipToolbar(const FontFamily& fontFamily, int fontSize, AssetsManager& assetsManager);

    void updatePos(const Vector2i& viewport);

private:
    GuiWidgetProgressBar* progressShields{nullptr};
    GuiWidgetProgressBar* progressHull{nullptr};
    GuiWidgetProgressBar* progressBattery{nullptr};
    GuiWidgetProgressBar* progressGenerated{nullptr};

    static const GuiStyleProgress styleProgressShields;
    static const GuiStyleProgress styleProgressHealth;
    static const GuiStyleProgress styleProgressBattery;
    static const GuiStyleProgress styleProgressGenerated;
};
} // namespace Engine
