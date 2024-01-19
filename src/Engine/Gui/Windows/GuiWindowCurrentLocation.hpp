#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API AssetsManager;

class ENGINE_API GuiWindowCurrentLocation : public GuiWindow {
public:
    explicit GuiWindowCurrentLocation(const FontFamily& fontFamily, int fontSize, AssetsManager& assetsManager);

    void updatePos(const Vector2i& viewport);

    void setSystemLabel(const std::string& value);
    void setSectorLabel(const std::string& value);

private:
    GuiWidgetLabel* systemLabel;
    GuiWidgetLabel* sectorLabel;
};
} // namespace Engine
