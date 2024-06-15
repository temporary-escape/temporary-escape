#include "GuiWindowMainMenu.hpp"

using namespace Engine;

GuiWindowMainMenu::GuiWindowMainMenu(GuiContext& ctx, const FontFamily& fontFamily, int fontSize) :
    GuiWindow{ctx, fontFamily, fontSize} {
    setSize({180.0f * 6.0f, 60.0f * 3.0f + 15.0f});
    setHeader(false);
    setDynamic(true);
    setBordered(false);
    setBackground(true);
    setStyle(guiStyleWindowNone);

    const float count = 6.0f;

    {
        auto& row = addWidget<GuiWidgetRow>(60.0f, count);

        buttonSingleplayer = &row.addWidget<GuiWidgetButtonToggle>("Singleplayer");
        buttonSingleplayer->setWidth(1.0f / count);
        buttonSingleplayer->setOnClick([this](const bool value) {
            if (value) {
                buttonMultiplayer->setValue(false);
            }
        });

        buttonMultiplayer = &row.addWidget<GuiWidgetButtonToggle>("Multiplayer");
        buttonMultiplayer->setWidth(1.0f / count);
        buttonMultiplayer->setOnClick([this](const bool value) {
            if (value) {
                buttonSingleplayer->setValue(false);
            }
        });

        buttonEditor = &row.addWidget<GuiWidgetButton>("Editor");
        buttonEditor->setWidth(1.0f / count);
        buttonEditor->setOnClick([this]() {
            resetButtons();
            if (onClickEditor) {
                onClickEditor();
            }
        });

        buttonSettings = &row.addWidget<GuiWidgetButton>("Settings");
        buttonSettings->setWidth(1.0f / count);
        buttonSettings->setOnClick([this]() {
            resetButtons();
            if (onClickSettings) {
                onClickSettings();
            }
        });

        buttonMods = &row.addWidget<GuiWidgetButton>("Mods");
        buttonMods->setWidth(1.0f / count);
        buttonMods->setOnClick([this]() {
            resetButtons();
            if (onClickMods) {
                onClickMods();
            }
        });

        buttonQuit = &row.addWidget<GuiWidgetButton>("Quit");
        buttonQuit->setWidth(1.0f / count);
        buttonQuit->setStyle(guiStyleButtonRedOutline);
    }

    {
        auto& row = addWidget<GuiWidgetRow>(60.0f, count);

        buttonNewGame = &row.addWidget<GuiWidgetButton>("New Game");
        buttonNewGame->setWidth(1.0f / count);
        buttonNewGame->setHidden(true);
        buttonNewGame->setOnClick([this]() {
            resetButtons();
            if (onClickNewGame) {
                onClickNewGame();
            }
        });

        buttonOnline = &row.addWidget<GuiWidgetButton>("Online");
        buttonOnline->setWidth(1.0f / count);
        buttonOnline->setHidden(true);
        buttonOnline->setOnClick([this]() {
            resetButtons();
            if (onClickOnline) {
                onClickOnline();
            }
        });
    }

    {
        auto& row = addWidget<GuiWidgetRow>(60.0f, count);

        buttonLoadSave = &row.addWidget<GuiWidgetButton>("Load Save");
        buttonLoadSave->setWidth(1.0f / count);
        buttonLoadSave->setHidden(true);
        buttonLoadSave->setOnClick([this]() {
            resetButtons();
            if (onClickLoadSave) {
                onClickLoadSave();
            }
        });

        buttonLocalLan = &row.addWidget<GuiWidgetButton>("Local LAN");
        buttonLocalLan->setWidth(1.0f / count);
        buttonLocalLan->setHidden(true);
        buttonLocalLan->setOnClick([this]() {
            resetButtons();
            if (onClickLocalLan) {
                onClickLocalLan();
            }
        });
    }
}

void GuiWindowMainMenu::update(const Vector2i& viewport) {
    GuiWindow::update(viewport);

    buttonNewGame->setHidden(!buttonSingleplayer->getValue());
    buttonLoadSave->setHidden(!buttonSingleplayer->getValue());
    buttonOnline->setHidden(!buttonMultiplayer->getValue());
    buttonLocalLan->setHidden(!buttonMultiplayer->getValue());

    const auto size = getSize();
    setPos({
        static_cast<float>(viewport.x) / 2.0f - size.x / 2.0f,
        static_cast<float>(viewport.y) - size.y - 30.0f,
    });
}

void GuiWindowMainMenu::resetButtons() {
    buttonSingleplayer->setValue(false);
    buttonMultiplayer->setValue(false);
}

void GuiWindowMainMenu::setOnClickQuit(GuiWidgetButton::OnClickCallback callback) {
    buttonQuit->setOnClick(std::move(callback));
}

void GuiWindowMainMenu::setOnClickSettings(GuiWidgetButton::OnClickCallback callback) {
    onClickSettings = std::move(callback);
}

void GuiWindowMainMenu::setOnClickNewGame(GuiWidgetButton::OnClickCallback callback) {
    onClickNewGame = std::move(callback);
}

void GuiWindowMainMenu::setOnClickLoadSave(GuiWidgetButton::OnClickCallback callback) {
    onClickLoadSave = std::move(callback);
}

void GuiWindowMainMenu::setOnClickOnline(GuiWidgetButton::OnClickCallback callback) {
    onClickOnline = std::move(callback);
}

void GuiWindowMainMenu::setOnClickLocalLan(GuiWidgetButton::OnClickCallback callback) {
    onClickLocalLan = std::move(callback);
}

void GuiWindowMainMenu::setOnClickEditor(GuiWidgetButton::OnClickCallback callback) {
    onClickEditor = std::move(callback);
}

void GuiWindowMainMenu::setOnClickMods(GuiWidgetButton::OnClickCallback callback) {
    onClickMods = std::move(callback);
}
