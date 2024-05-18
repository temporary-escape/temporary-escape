#include "GuiWidget.hpp"

using namespace Engine;

GuiWidget::GuiWidget(GuiContext& ctx) : ctx{ctx} {
}

void GuiWidget::draw() {
    beforeDraw();

    if (!tooltip.empty()) {
        ctx.tooltip(tooltip);
    }

    if (onHover) {
        if (!hovered && ctx.isHovered()) {
            hovered = true;
            onHover(true);
        } else if (hovered && !ctx.isHovered()) {
            hovered = false;
            onHover(false);
        }
    }

    drawInternal();
}

void GuiWidget::drawInternal() {
    ctx.skip();
}

void GuiWidget::setWidth(const float value, const bool pixels) {
    width = value;
    widthPixels = pixels;
}

void GuiWidget::setTooltip(std::string value) {
    tooltip = std::move(value);
}

void GuiWidget::setOnHover(OnHoverCallback value) {
    onHover = std::move(value);
}
