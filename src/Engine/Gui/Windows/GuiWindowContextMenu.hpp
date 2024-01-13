#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowContextMenu : public GuiWindow {
public:
    using NestedCallback = std::function<void(GuiWindowContextMenu&)>;

    GuiWindowContextMenu(const FontFamily& fontFamily, int fontSize, GuiWindowContextMenu* child);

    void update(const Vector2i& viewport) override;

    GuiWidgetContextButton& addItem(ImagePtr image, std::string label,
                                    GuiWidgetContextButton::OnClickCallback callback);
    void addNested(ImagePtr imageLeft, ImagePtr imageRight, std::string label, NestedCallback callback);
    void clear();

private:
    GuiWindowContextMenu* child{nullptr};
    GuiWidgetRow* row{nullptr};
};
} // namespace Engine
