#include "gui_context_menu.hpp"

using namespace Engine;

GuiContextMenu::GuiContextMenu() {
    setSize({200.0f, 400.0f});
    setFlags(Nuklear::WindowFlags::Dynamic | Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Border |
             Nuklear::WindowFlags::NoInput);
}

void GuiContextMenu::drawLayout(Nuklear& nuklear) {
    for (const auto& item : items) {
        nuklear.layoutDynamic(25.0f, 1);
        if (nuklear.button(item.label, Nuklear::TextAlign::Left)) {
            item.callback();
            setEnabled(false);
        }
    }
}
