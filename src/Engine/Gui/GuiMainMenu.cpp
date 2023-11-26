#include "GuiMainMenu.hpp"

using namespace Engine;

GuiMainMenu::GuiMainMenu() {
    setSize({300.0f, 400.0f});
    setFlags(Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Dynamic | Nuklear::WindowFlags::Border |
             Nuklear::WindowFlags::Title);
    setTitle("Main Menu");
}

void GuiMainMenu::drawLayout(Nuklear& nuklear) {
    for (const auto& item : items) {
        nuklear.layoutDynamic(45.0f, 1);
        if (nuklear.button(item.label, Nuklear::TextAlign::Center)) {
            item.callback();
        }
    }
}

void GuiMainMenu::beforeDraw(Nuklear& nuklear, const Vector2& viewport) {
    setPos({viewport.x / 2 - getSize().x / 2, viewport.y / 2 - getSize().y / 2});
}
