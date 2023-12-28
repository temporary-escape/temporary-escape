#include "GuiWidgetProgressBar.hpp"

using namespace Engine;

GuiWidgetProgressBar::GuiWidgetProgressBar(GuiContext& ctx) : GuiWidget{ctx}, progress{0.0f} {
}

void GuiWidgetProgressBar::drawInternal() {
    ctx.progress(progress, max, color);
}

void GuiWidgetProgressBar::setValue(const float value) {
    progress = value;
    ctx.setDirty();
}

void GuiWidgetProgressBar::setMax(const float value) {
    max = value;
    ctx.setDirty();
}

void GuiWidgetProgressBar::setColor(const Color4& value) {
    color = value;
    ctx.setDirty();
}
