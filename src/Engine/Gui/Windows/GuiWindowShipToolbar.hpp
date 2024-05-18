#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API AssetsManager;

class ENGINE_API GuiWindowShipToolbar : public GuiWindow {
public:
    explicit GuiWindowShipToolbar(GuiContext& ctx, const FontFamily& fontFamily, int fontSize,
                                  AssetsManager& assetsManager);

    void update(const Vector2i& viewport) override;
};
} // namespace Engine
