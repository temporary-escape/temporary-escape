#include "GuiWidgetTemplateRow.hpp"

using namespace Engine;

GuiWidgetTemplateRow::GuiWidgetTemplateRow(GuiContext& ctx, const float height) : GuiWidgetLayout{ctx}, height{height} {
}

void GuiWidgetTemplateRow::draw() {
    ctx.layoutTemplateBegin(height);
    for (auto& widget : widgets) {
        if (widget->isWidthPixels()) {
            ctx.layoutTemplatePushStatic(widget->getWidth());
        } else if (widget->getWidth() == 0.0f) {
            ctx.layoutTemplatePushDynamic();
        } else {
            ctx.layoutTemplatePushVariable(widget->getWidth());
        }
    }
    ctx.layoutTemplateEnd();

    for (auto& widget : widgets) {
        widget->draw();
    }
}

void GuiWidgetTemplateRow::setHeight(const float value) {
    height = value;
}
