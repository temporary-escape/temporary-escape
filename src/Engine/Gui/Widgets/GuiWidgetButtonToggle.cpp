#include "GuiWidgetButtonToggle.hpp"

using namespace Engine;

GuiWidgetButtonToggle::GuiWidgetButtonToggle(GuiContext& ctx, std::string label) :
    GuiWidget{ctx}, label{std::move(label)}, style{&guiStyleButtonYellowOutline} {
}

void GuiWidgetButtonToggle::drawInternal() {
    if (ctx.buttonToggle(label, getStyle(), toggle)) {
        if (onClick) {
            onClick(toggle);
        }
    }
}

void GuiWidgetButtonToggle::setLabel(std::string value) {
    label = std::move(value);
}

void GuiWidgetButtonToggle::setOnClick(OnClickCallback value) {
    onClick = std::move(value);
}

void GuiWidgetButtonToggle::setValue(const bool value) {
    toggle = value;
}

void GuiWidgetButtonToggle::setStyle(const GuiStyleButton& value) {
    style = &value;
}
