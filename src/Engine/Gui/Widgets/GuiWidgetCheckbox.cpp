#include "GuiWidgetCheckbox.hpp"

using namespace Engine;

GuiWidgetCheckbox::GuiWidgetCheckbox(GuiContext& ctx, std::string label, bool& value) :
    GuiWidget{ctx}, label{std::move(label)}, value{&value} {
}

void GuiWidgetCheckbox::drawInternal() {
    if (ctx.checkbox(label, *value)) {
        if (onClick) {
            onClick(*value);
        }
    }
}

void GuiWidgetCheckbox::setLabel(std::string value) {
    label = std::move(value);
}

void GuiWidgetCheckbox::setOnClick(OnClickCallback value) {
    onClick = std::move(value);
}
