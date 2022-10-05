#include "GuiBlockSelector.hpp"

using namespace Engine;

GuiBlockSelector::GuiBlockSelector(Nuklear& nuklear) : GuiWindow{nuklear} {
    setSize({700.0f, 350.0f});
    setTitle("Block Browser");
}

void GuiBlockSelector::drawLayout() {
}

void GuiBlockSelector::beforeDraw(const Vector2i& viewport) {
    setPos({0.0f, viewport.y - getSize().y});
}
