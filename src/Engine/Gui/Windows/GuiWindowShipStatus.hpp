#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API AssetsManager;

class ENGINE_API GuiWindowShipStatus : public GuiWindow {
public:
    explicit GuiWindowShipStatus(GuiContext& ctx, const FontFamily& fontFamily, int fontSize,
                                 AssetsManager& assetsManager);

    void update(const Vector2i& viewport) override;

private:
    GuiWidgetProgressBar* progressShields{nullptr};
    GuiWidgetProgressBar* progressHealth{nullptr};
    GuiWidgetProgressBar* progressEnergy{nullptr};

    static const GuiStyleProgress styleProgressShields;
    static const GuiStyleProgress styleProgressHealth;
    static const GuiStyleProgress styleProgressEnergy;
};
} // namespace Engine
