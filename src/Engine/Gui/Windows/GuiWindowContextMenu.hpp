#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowContextMenu : public GuiWindow {
public:
    GuiWindowContextMenu(const FontFamily& fontFamily, int fontSize);

    GuiWidgetButton& addItem(std::string label);
    void clear();

private:
    GuiWidgetRow* row{nullptr};
};
} // namespace Engine
