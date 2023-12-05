#include "GuiWindowMainMenu.hpp"

using namespace Engine;

GuiWindowMainMenu::GuiWindowMainMenu(const FontFamily& fontFamily, int fontSize) :
    GuiWindow2{fontFamily, static_cast<int>(fontSize * 1.5f)} {
    setSize({350.0f, 600.0f});
    setTitle("Main Menu");

    auto& row = addWidget<GuiWidgetRow>(60.0f, 1);
    row.addWidget<GuiWidgetButton>("Hello World!");
    row.addWidget<GuiWidgetButton>("Settings");
    row.addWidget<GuiWidgetButton>("Quit");
}
