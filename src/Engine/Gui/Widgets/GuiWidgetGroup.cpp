#include "GuiWidgetGroup.hpp"

using namespace Engine;

GuiWidgetGroup::GuiWidgetGroup(GuiContext& ctx) :
    GuiWidgetLayout{ctx}, name{std::to_string(reinterpret_cast<uint64_t>(this))}, style{&guiStyleGroupDefault} {
}

void GuiWidgetGroup::draw() {
    if (ctx.groupBegin(name, *style, scrollbar, false)) {
        GuiWidgetLayout::drawInternal();
        ctx.groupEnd();
    }
}

void GuiWidgetGroup::setScrollbar(const bool value) {
    scrollbar = value;
}

void GuiWidgetGroup::setStyle(const GuiStyleGroup& value) {
    style = &value;
}
