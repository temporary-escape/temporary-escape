#include "GuiWidgetButton.hpp"

using namespace Engine;

GuiWidgetButton::GuiWidgetButton(GuiContext& ctx, std::string label) :
    GuiWidget{ctx}, label{std::move(label)}, style{&guiStyleButtonYellowOutline} {
}

void GuiWidgetButton::drawInternal() {
    if (ctx.button(label, getStyle(), image)) {
        if (onClick) {
            onClick();
        }
    }
}

void GuiWidgetButton::setLabel(std::string value) {
    label = std::move(value);
}

void GuiWidgetButton::setImage(ImagePtr value) {
    image = std::move(value);
}

void GuiWidgetButton::setOnClick(OnClickCallback value) {
    onClick = std::move(value);
}

void GuiWidgetButton::setStyle(const GuiStyleButton& value) {
    style = &value;
}
