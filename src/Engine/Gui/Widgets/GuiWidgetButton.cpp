#include "GuiWidgetButton.hpp"

using namespace Engine;

GuiWidgetButton::GuiWidgetButton(GuiContext& ctx, std::string label) : GuiWidget{ctx}, label{std::move(label)} {
}

void GuiWidgetButton::draw() {
    if (ctx.button(label)) {
        if (onClick) {
            onClick();
        }
    }
}

void GuiWidgetButton::setLabel(std::string value) {
    label = std::move(value);
}

void GuiWidgetButton::setOnClick(OnClickCallback value) {
    onClick = std::move(value);
}
