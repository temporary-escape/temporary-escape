#include "GuiWidgetContextButton.hpp"

using namespace Engine;

GuiWidgetContextButton::GuiWidgetContextButton(GuiContext& ctx, ImagePtr image, std::string label) :
    GuiWidget{ctx}, image{std::move(image)}, label{std::move(label)} {
}

void GuiWidgetContextButton::drawInternal() {
    if (ctx.contextButton(label, image)) {
        if (onClick) {
            onClick();
        }
    }
}

void GuiWidgetContextButton::setLabel(std::string value) {
    label = std::move(value);
    ctx.setDirty();
}

void GuiWidgetContextButton::setImage(ImagePtr value) {
    image = std::move(value);
    ctx.setDirty();
}

void GuiWidgetContextButton::setOnClick(OnClickCallback value) {
    onClick = std::move(value);
}
