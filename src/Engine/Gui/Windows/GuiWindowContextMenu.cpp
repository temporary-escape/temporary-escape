#include "GuiWindowContextMenu.hpp"

using namespace Engine;

GuiWindowContextMenu::GuiWindowContextMenu(const FontFamily& fontFamily, int fontSize) :
    GuiWindow{fontFamily, fontSize} {
    setSize({200.0f, 600.0f});
    setCentered(false);
    setTitle("Context Menu");
    setDynamic(true);
    setHeader(false);
    setNoScrollbar(true);

    row = &addWidget<GuiWidgetRow>(fontSize, 1);
}

GuiWidgetButton& GuiWindowContextMenu::addItem(std::string label) {
    return row->addWidget<GuiWidgetButton>(std::move(label));
}

void GuiWindowContextMenu::clear() {
    row->clearWidgets();
}
