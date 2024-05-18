#include "GuiWindowMainMenu.hpp"

using namespace Engine;

GuiWindowMainMenu::GuiWindowMainMenu(GuiContext& ctx, const FontFamily& fontFamily, int fontSize) :
    GuiWindow{ctx, fontFamily, fontSize} {
    setSize({250.0f, 500.0f});
    setTitle("MAIN MENU");
    setDynamic(true);

    auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
    buttonNewGame = &row.addWidget<GuiWidgetButton>("New Game");
    buttonNewGame->setStyle(&GuiWidgetButton::menuStyle);
    buttonLoadSave = &row.addWidget<GuiWidgetButton>("Load Save");
    buttonLoadSave->setStyle(&GuiWidgetButton::menuStyle);
    buttonMultiplayer = &row.addWidget<GuiWidgetButton>("Multiplayer");
    buttonMultiplayer->setStyle(&GuiWidgetButton::menuStyle);
    buttonEditor = &row.addWidget<GuiWidgetButton>("Editor");
    buttonEditor->setStyle(&GuiWidgetButton::menuStyle);
    buttonSettings = &row.addWidget<GuiWidgetButton>("Settings");
    buttonSettings->setStyle(&GuiWidgetButton::menuStyle);
    buttonMods = &row.addWidget<GuiWidgetButton>("Mods");
    buttonMods->setStyle(&GuiWidgetButton::menuStyle);
    buttonQuit = &row.addWidget<GuiWidgetButton>("Quit");
    buttonQuit->setStyle(&GuiWidgetButton::menuStyle);
}

void GuiWindowMainMenu::setOnClickQuit(GuiWidgetButton::OnClickCallback callback) {
    buttonQuit->setOnClick(std::move(callback));
}

void GuiWindowMainMenu::setOnClickSettings(GuiWidgetButton::OnClickCallback callback) {
    buttonSettings->setOnClick(std::move(callback));
}

void GuiWindowMainMenu::setOnClickNewGame(GuiWidgetButton::OnClickCallback callback) {
    buttonNewGame->setOnClick(std::move(callback));
}

void GuiWindowMainMenu::setOnClickLoadSave(GuiWidgetButton::OnClickCallback callback) {
    buttonLoadSave->setOnClick(std::move(callback));
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
