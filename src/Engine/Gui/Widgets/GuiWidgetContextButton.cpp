#include "GuiWidgetContextButton.hpp"

using namespace Engine;

GuiWidgetContextButton::GuiWidgetContextButton(GuiContext& ctx, ImagePtr imageLeft, ImagePtr imageRight,
                                               std::string label) :
    GuiWidget{ctx}, imageLeft{std::move(imageLeft)}, imageRight{std::move(imageRight)}, label{std::move(label)} {
}

void GuiWidgetContextButton::drawInternal() {
    if (ctx.contextButton(label, imageLeft, imageRight)) {
        if (onClick) {
            onClick();
        }
    }
}

void GuiWidgetContextButton::setLabel(std::string value) {
    label = std::move(value);
}

void GuiWidgetContextButton::setImageLeft(ImagePtr value) {
    imageLeft = std::move(value);
}

void GuiWidgetContextButton::setImageRight(ImagePtr value) {
    imageRight = std::move(value);
}

void GuiWidgetContextButton::setOnClick(OnClickCallback value) {
    onClick = std::move(value);
}
