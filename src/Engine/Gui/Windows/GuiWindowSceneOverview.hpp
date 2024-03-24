#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowSceneOverview : public GuiWindow {
public:
    GuiWindowSceneOverview(const FontFamily& fontFamily, int fontSize);

    void updatePos(const Vector2i& viewport);

private:
};
} // namespace Engine
