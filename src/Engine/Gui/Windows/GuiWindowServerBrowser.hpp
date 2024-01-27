#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiManager;

class ENGINE_API GuiWindowServerBrowser : public GuiWindow {
public:
    explicit GuiWindowServerBrowser(const FontFamily& fontFamily, int fontSize);

    void setOnClickClose(GuiWidgetButton::OnClickCallback callback);

private:
    GuiWidgetButton* buttonClose;
};
} // namespace Engine
