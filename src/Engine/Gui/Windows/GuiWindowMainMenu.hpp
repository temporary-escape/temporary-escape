#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowMainMenu : public GuiWindow {
public:
    GuiWindowMainMenu(GuiContext& ctx, const FontFamily& fontFamily, int fontSize);

    void setOnClickNewGame(GuiWidgetButton::OnClickCallback callback);
    void setOnClickLoadSave(GuiWidgetButton::OnClickCallback callback);
    void setOnClickMultiplayer(GuiWidgetButton::OnClickCallback callback);
    void setOnClickEditor(GuiWidgetButton::OnClickCallback callback);
    void setOnClickSettings(GuiWidgetButton::OnClickCallback callback);
    void setOnClickMods(GuiWidgetButton::OnClickCallback callback);
    void setOnClickQuit(GuiWidgetButton::OnClickCallback callback);

private:
    GuiWidgetButton* buttonNewGame;
    GuiWidgetButton* buttonLoadSave;
    GuiWidgetButton* buttonMultiplayer;
    GuiWidgetButton* buttonEditor;
    GuiWidgetButton* buttonSettings;
    GuiWidgetButton* buttonMods;
    GuiWidgetButton* buttonQuit;
};
} // namespace Engine
