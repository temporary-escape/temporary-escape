#include "GuiWindowSceneOverview.hpp"

using namespace Engine;

GuiWindowSceneOverview::GuiWindowSceneOverview(const FontFamily& fontFamily, int fontSize) :
    GuiWindow{fontFamily, fontSize} {
    setSize({350.0f, 500.0f});
    setOpacity(0.7f);
    setCentered(false);
    setTitle("Overview");
}

void GuiWindowSceneOverview::updatePos(const Vector2i& viewport) {
    setPos({static_cast<float>(viewport.x) - getSize().x, 0.0f});
    setSize({getSize().x, viewport.y});
}
