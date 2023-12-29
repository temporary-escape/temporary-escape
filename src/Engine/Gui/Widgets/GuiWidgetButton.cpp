#include "GuiWidgetButton.hpp"

using namespace Engine;

GuiWidgetButton::GuiWidgetButton(GuiContext& ctx, std::string label) : GuiWidget{ctx}, label{std::move(label)} {
}

void GuiWidgetButton::drawInternal() {
    if (ctx.button(label, image)) {
        if (onClick) {
            onClick();
        }
    }
}

void GuiWidgetButton::setLabel(std::string value) {
    label = std::move(value);
    ctx.setDirty();
}

void GuiWidgetButton::setImage(ImagePtr value) {
    image = std::move(value);
    ctx.setDirty();
}

void GuiWidgetButton::setOnClick(OnClickCallback value) {
    onClick = std::move(value);
}
