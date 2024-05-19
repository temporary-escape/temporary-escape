#include "GuiWindowGameMenu.hpp"

using namespace Engine;

GuiWindowGameMenu::GuiWindowGameMenu(GuiContext& ctx, const FontFamily& fontFamily, int fontSize) :
    GuiWindow{ctx, fontFamily, fontSize} {
    setSize({180.0f * 4.0f + 10.0f, 60.0f * 1.0f + 45.0f});
    setTitle("GAME MENU");
    setDynamic(true);
    setEnabled(false);
    setHeaderPrimary(true);

    const float count = 4.0f;

    {
        auto& row = addWidget<GuiWidgetRow>(60.0f, count);

        buttonContinue = &row.addWidget<GuiWidgetButton>("Continue");
        buttonContinue->setWidth(1.0f / count);
        buttonContinue->setStyle(&GuiWidgetButton::successStyle);

        buttonSettings = &row.addWidget<GuiWidgetButton>("Settings");
        buttonSettings->setWidth(1.0f / count);

        buttonQuitToMenu = &row.addWidget<GuiWidgetButton>("Exit to menu");
        buttonQuitToMenu->setWidth(1.0f / count);

        buttonQuitGame = &row.addWidget<GuiWidgetButton>("Quit Game");
        buttonQuitGame->setWidth(1.0f / count);
    }
}

void GuiWindowGameMenu::update(const Vector2i& viewport) {
    GuiWindow::update(viewport);
}

void GuiWindowGameMenu::setOnClickContinue(GuiWidgetButton::OnClickCallback callback) {
    buttonContinue->setOnClick(std::move(callback));
}

void GuiWindowGameMenu::setOnClickSettings(GuiWidgetButton::OnClickCallback callback) {
    buttonSettings->setOnClick(std::move(callback));
}

void GuiWindowGameMenu::setOnClickQuitToMenu(GuiWidgetButton::OnClickCallback callback) {
    buttonQuitToMenu->setOnClick(std::move(callback));
}

void GuiWindowGameMenu::setOnClickQuitGame(GuiWidgetButton::OnClickCallback callback) {
    buttonQuitGame->setOnClick(std::move(callback));
}
