#include "GuiWidgetLabel.hpp"

using namespace Engine;

GuiWidgetLabel::GuiWidgetLabel(GuiContext& ctx, std::string label) :
    GuiWidget{ctx}, label{std::move(label)}, color{Colors::text} {
}

void GuiWidgetLabel::drawInternal() {
    ctx.label(label, color);
}

void GuiWidgetLabel::setLabel(std::string value) {
    label = std::move(value);
}

void GuiWidgetLabel::setColor(const Color4& value) {
    color = value;
}
