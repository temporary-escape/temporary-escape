#include "GuiWidgetRow.hpp"

using namespace Engine;

GuiWidgetRow::GuiWidgetRow(GuiContext& ctx, const float height, const int columns) :
    GuiWidgetLayout{ctx}, height{height}, columns{columns} {
}

void GuiWidgetRow::draw() {
    ctx.layoutRowBegin(height, columns);
    for (auto& widget : widgets) {
        ctx.layoutRowPush(widget->getWidth());
        widget->draw();
    }
    ctx.layoutRowEnd();
}

void GuiWidgetRow::setHeight(const float value) {
    height = value;
}

void GuiWidgetRow::setColumns(const int value) {
    columns = value;
}
