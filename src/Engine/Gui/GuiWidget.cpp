#include "GuiWidget.hpp"

using namespace Engine;

GuiWidget::GuiWidget(GuiContext& ctx) : ctx{ctx} {
}

void GuiWidget::draw() {
    beforeDraw();

    if (!tooltip.empty()) {
        ctx.tooltip(tooltip);
    }

    drawInternal();
}

void GuiWidget::drawInternal() {
    ctx.skip();
}

void GuiWidget::setWidth(const float value, const bool pixels) {
    width = value;
    widthPixels = pixels;
    ctx.setDirty();
}

void GuiWidget::setTooltip(std::string value) {
    tooltip = std::move(value);
    ctx.setDirty();
}
