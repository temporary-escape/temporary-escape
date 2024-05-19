#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowMainMenu : public GuiWindow {
public:
    GuiWindowMainMenu(GuiContext& ctx, const FontFamily& fontFamily, int fontSize);

    void setOnClickNewGame(GuiWidgetButton::OnClickCallback callback);
    void setOnClickLoadSave(GuiWidgetButton::OnClickCallback callback);
    void setOnClickServerBrowser(GuiWidgetButton::OnClickCallback callback);
    void setOnClickEditor(GuiWidgetButton::OnClickCallback callback);
    void setOnClickSettings(GuiWidgetButton::OnClickCallback callback);
    void setOnClickMods(GuiWidgetButton::OnClickCallback callback);
    void setOnClickQuit(GuiWidgetButton::OnClickCallback callback);

    void update(const Vector2i& viewport) override;

private:
    void resetButtons();

    GuiWidgetButtonToggle* buttonSingleplayer;
    GuiWidgetButton* buttonNewGame;
    GuiWidgetButton* buttonLoadSave;
    GuiWidgetButtonToggle* buttonMultiplayer;
    GuiWidgetButton* buttonServerBrowser;
    GuiWidgetButton* buttonEditor;
    GuiWidgetButton* buttonSettings;
    GuiWidgetButton* buttonMods;
    GuiWidgetButton* buttonQuit;

    GuiWidgetButton::OnClickCallback onClickEditor;
    GuiWidgetButton::OnClickCallback onClickSettings;
    GuiWidgetButton::OnClickCallback onClickMods;
    GuiWidgetButton::OnClickCallback onClickNewGame;
    GuiWidgetButton::OnClickCallback onClickLoadSave;
    GuiWidgetButton::OnClickCallback onClickServerBrowser;
};
} // namespace Engine
