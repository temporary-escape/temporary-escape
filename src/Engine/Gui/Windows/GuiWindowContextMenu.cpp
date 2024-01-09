#include "GuiWindowContextMenu.hpp"

using namespace Engine;

GuiWindowContextMenu::GuiWindowContextMenu(const FontFamily& fontFamily, int fontSize) :
    GuiWindow{fontFamily, fontSize} {
    setSize({200.0f, 600.0f});
    setCentered(false);
    setTitle("Context Menu");
    setDynamic(true);
    setHeader(false);
    setNoScrollbar(true);

    row = &addWidget<GuiWidgetRow>(static_cast<float>(fontSize) * 1.25f, 1);
}

GuiWidgetContextButton& GuiWindowContextMenu::addItem(ImagePtr image, std::string label,
                                                      GuiWidgetContextButton::OnClickCallback callback) {
    auto& button = row->addWidget<GuiWidgetContextButton>(std::move(image), std::move(label));
    button.setOnClick(std::move(callback));
    return button;
}

void GuiWindowContextMenu::clear() {
    row->clearWidgets();
}
