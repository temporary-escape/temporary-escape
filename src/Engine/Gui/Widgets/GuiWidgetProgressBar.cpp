#include "GuiWidgetProgressBar.hpp"

using namespace Engine;

GuiWidgetProgressBar::GuiWidgetProgressBar(GuiContext& ctx) :
    GuiWidget{ctx}, progress{0.0f}, style{&guiStyleProgressYellow} {
}

void GuiWidgetProgressBar::drawInternal() {
    ctx.progress(progress, max, *style, height);
}

void GuiWidgetProgressBar::setValue(const float value) {
    progress = value;
}

void GuiWidgetProgressBar::setMax(const float value) {
    max = value;
}

void GuiWidgetProgressBar::setHeight(const float value) {
    height = value;
}

void GuiWidgetProgressBar::setStyle(const GuiStyleProgress& value) {
    style = &value;
}
