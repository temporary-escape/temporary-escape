#include "GuiWidgetProgressBar.hpp"

using namespace Engine;

const GuiStyleProgress GuiWidgetProgressBar::defaultStyle{
    .border =
        {
            .normal = Colors::border,
            .hover = Colors::border,
            .active = Colors::border,
        },
    .bar =
        {
            .normal = Colors::text,
            .hover = Colors::text,
            .active = Colors::text,
        },
};

GuiWidgetProgressBar::GuiWidgetProgressBar(GuiContext& ctx) : GuiWidget{ctx}, progress{0.0f} {
}

void GuiWidgetProgressBar::drawInternal() {
    ctx.progress(progress, max, style ? *style : defaultStyle, height);
}

void GuiWidgetProgressBar::setValue(const float value) {
    progress = value;
    ctx.setDirty();
}

void GuiWidgetProgressBar::setMax(const float value) {
    max = value;
    ctx.setDirty();
}

void GuiWidgetProgressBar::setHeight(const float value) {
    height = value;
    ctx.setDirty();
}

void GuiWidgetProgressBar::setStyle(const GuiStyleProgress* value) {
    style = value;
    ctx.setDirty();
}
