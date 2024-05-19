#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowGameMenu : public GuiWindow {
public:
    GuiWindowGameMenu(GuiContext& ctx, const FontFamily& fontFamily, int fontSize);

    void setOnClickContinue(GuiWidgetButton::OnClickCallback callback);
    void setOnClickSettings(GuiWidgetButton::OnClickCallback callback);
    void setOnClickQuitToMenu(GuiWidgetButton::OnClickCallback callback);
    void setOnClickQuitGame(GuiWidgetButton::OnClickCallback callback);

    void update(const Vector2i& viewport) override;

private:
    GuiWidgetButton* buttonContinue;
    GuiWidgetButton* buttonSettings;
    GuiWidgetButton* buttonQuitToMenu;
    GuiWidgetButton* buttonQuitGame;
};
} // namespace Engine
