#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API AssetsManager;

class ENGINE_API GuiWindowShipStatus : public GuiWindow {
public:
    explicit GuiWindowShipStatus(const FontFamily& fontFamily, int fontSize, AssetsManager& assetsManager);

    void updatePos(const Vector2i& viewport);

private:
    GuiWidgetProgressBar* progressShields{nullptr};
    GuiWidgetProgressBar* progressHealth{nullptr};
    GuiWidgetProgressBar* progressEnergy{nullptr};

    static const GuiStyleProgress styleProgressShields;
    static const GuiStyleProgress styleProgressHealth;
    static const GuiStyleProgress styleProgressEnergy;
};
} // namespace Engine
