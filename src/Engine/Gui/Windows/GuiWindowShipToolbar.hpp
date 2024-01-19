#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API AssetsManager;

class ENGINE_API GuiWindowShipToolbar : public GuiWindow {
public:
    explicit GuiWindowShipToolbar(const FontFamily& fontFamily, int fontSize, AssetsManager& assetsManager);

    void updatePos(const Vector2i& viewport);
};
} // namespace Engine
