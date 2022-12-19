#include "gui_block_selector.hpp"

using namespace Engine;

GuiBlockSelector::GuiBlockSelector() {
    setSize({700.0f, 350.0f});
    setTitle("Block Browser");
}

void GuiBlockSelector::drawLayout(Nuklear& nuklear) {
}

void GuiBlockSelector::beforeDraw(const Vector2& viewport) {
    setPos({0.0f, viewport.y - getSize().y});
}
