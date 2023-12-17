#include "GuiWidgetCheckbox.hpp"

using namespace Engine;

GuiWidgetCheckbox::GuiWidgetCheckbox(GuiContext& ctx, std::string label) : GuiWidget{ctx}, label{std::move(label)} {
}

void GuiWidgetCheckbox::draw() {
    if (ctx.checkbox(label, toggle)) {
        if (onClick) {
            onClick(toggle);
        }
    }
}

void GuiWidgetCheckbox::setLabel(std::string value) {
    label = std::move(value);
}

void GuiWidgetCheckbox::setOnClick(OnClickCallback value) {
    onClick = std::move(value);
}

void GuiWidgetCheckbox::setValue(const bool value) {
    toggle = value;
}
