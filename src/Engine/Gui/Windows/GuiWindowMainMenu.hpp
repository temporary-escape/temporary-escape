#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowMainMenu : public GuiWindow {
public:
    GuiWindowMainMenu(const FontFamily& fontFamily, int fontSize);

    void setOnClickSingleplayer(GuiWidgetButton::OnClickCallback callback);
    void setOnClickMultiplayer(GuiWidgetButton::OnClickCallback callback);
    void setOnClickEditor(GuiWidgetButton::OnClickCallback callback);
    void setOnClickSettings(GuiWidgetButton::OnClickCallback callback);
    void setOnClickMods(GuiWidgetButton::OnClickCallback callback);
    void setOnClickQuit(GuiWidgetButton::OnClickCallback callback);

private:
    GuiWidgetButton* buttonSingleplayer;
    GuiWidgetButton* buttonMultiplayer;
    GuiWidgetButton* buttonEditor;
    GuiWidgetButton* buttonSettings;
    GuiWidgetButton* buttonMods;
    GuiWidgetButton* buttonQuit;
};
} // namespace Engine
