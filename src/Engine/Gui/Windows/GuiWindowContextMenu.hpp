#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowContextMenu : public GuiWindow {
public:
    GuiWindowContextMenu(const FontFamily& fontFamily, int fontSize);

    GuiWidgetContextButton& addItem(ImagePtr image, std::string label,
                                    GuiWidgetContextButton::OnClickCallback callback);
    void clear();

private:
    GuiWidgetRow* row{nullptr};
};
} // namespace Engine
