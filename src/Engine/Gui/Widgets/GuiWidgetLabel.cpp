#include "GuiWidgetLabel.hpp"

using namespace Engine;

GuiWidgetLabel::GuiWidgetLabel(GuiContext& ctx, std::string label) : GuiWidget{ctx}, label{std::move(label)} {
}

void GuiWidgetLabel::draw() {
}

void GuiWidgetLabel::setLabel(std::string value) {
    label = std::move(value);
}
