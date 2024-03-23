#include "GuiWindowSinglePlayerMenu.hpp"

using namespace Engine;

GuiWindowSinglePlayerMenu::GuiWindowSinglePlayerMenu(const FontFamily& fontFamily, int fontSize) :
    GuiWindow{fontFamily, fontSize} {
    setSize({250.0f, 600.0f});
    setTitle("Single Player");
    setDynamic(true);

    auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
    buttonCreate = &row.addWidget<GuiWidgetButton>("New Game");
    buttonLoad = &row.addWidget<GuiWidgetButton>("Load Save");
    buttonBack = &row.addWidget<GuiWidgetButton>("Back");
}

void GuiWindowSinglePlayerMenu::setOnCreate(GuiWidgetButton::OnClickCallback callback) {
    buttonCreate->setOnClick(std::move(callback));
}

void GuiWindowSinglePlayerMenu::setOnLoad(GuiWidgetButton::OnClickCallback callback) {
    buttonLoad->setOnClick(std::move(callback));
}

void GuiWindowSinglePlayerMenu::setOnClose(GuiWidgetButton::OnClickCallback callback) {
    buttonBack->setOnClick(std::move(callback));
}
