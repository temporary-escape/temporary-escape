#include "GuiWidgetProgressBar.hpp"

using namespace Engine;

const GuiStyleProgress GuiWidgetProgressBar::defaultStyle{
    {
        Colors::border,
        Colors::border,
        Colors::border,
    },
    {
        Colors::text,
        Colors::text,
        Colors::text,
    },
};

GuiWidgetProgressBar::GuiWidgetProgressBar(GuiContext& ctx) : GuiWidget{ctx}, progress{0.0f} {
}

void GuiWidgetProgressBar::drawInternal() {
    ctx.progress(progress, max, style ? *style : defaultStyle, height);
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

void GuiWidgetProgressBar::setStyle(const GuiStyleProgress* value) {
    style = value;
}
