#include "GuiWindowMainMenu.hpp"

using namespace Engine;

GuiWindowMainMenu::GuiWindowMainMenu(const FontFamily& fontFamily, int fontSize) : GuiWindow2{fontFamily, fontSize} {
    setSize({250.0f, 600.0f});
    setTitle("Main Menu");
    setDynamic(true);

    auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
    buttonSingleplayer = &row.addWidget<GuiWidgetButton>("Singleplayer");
    buttonMultiplayer = &row.addWidget<GuiWidgetButton>("Multiplayer");
    buttonEditor = &row.addWidget<GuiWidgetButton>("Editor");
    buttonSettings = &row.addWidget<GuiWidgetButton>("Settings");
    buttonMods = &row.addWidget<GuiWidgetButton>("Mods");
    buttonQuit = &row.addWidget<GuiWidgetButton>("Quit");
}

void GuiWindowMainMenu::setOnClickQuit(GuiWidgetButton::OnClickCallback callback) {
    buttonQuit->setOnClick(std::move(callback));
}

void GuiWindowMainMenu::setOnClickSettings(GuiWidgetButton::OnClickCallback callback) {
    buttonSettings->setOnClick(std::move(callback));
}

void GuiWindowMainMenu::setOnClickSingleplayer(GuiWidgetButton::OnClickCallback callback) {
    buttonSingleplayer->setOnClick(std::move(callback));
}

void GuiWindowMainMenu::setOnClickMultiplayer(GuiWidgetButton::OnClickCallback callback) {
    buttonMultiplayer->setOnClick(std::move(callback));
}

void GuiWindowMainMenu::setOnClickEditor(GuiWidgetButton::OnClickCallback callback) {
    buttonEditor->setOnClick(std::move(callback));
}

void GuiWindowMainMenu::setOnClickMods(GuiWidgetButton::OnClickCallback callback) {
    buttonMods->setOnClick(std::move(callback));
}
