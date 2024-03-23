#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowSinglePlayerMenu : public GuiWindow {
public:
    GuiWindowSinglePlayerMenu(const FontFamily& fontFamily, int fontSize);

    void setOnCreate(GuiWidgetButton::OnClickCallback callback);
    void setOnLoad(GuiWidgetButton::OnClickCallback callback);
    void setOnClose(GuiWidgetButton::OnClickCallback callback);

private:
    GuiWidgetButton* buttonCreate;
    GuiWidgetButton* buttonLoad;
    GuiWidgetButton* buttonBack;
};
} // namespace Engine
