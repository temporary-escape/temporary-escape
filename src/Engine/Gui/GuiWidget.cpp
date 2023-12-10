#include "GuiWidget.hpp"

using namespace Engine;

GuiWidget::GuiWidget(GuiContext& ctx) : ctx{ctx} {
}

void GuiWidget::draw() {
    ctx.skip();
}

void GuiWidget::setWidth(const float value) {
    width = value;
}
