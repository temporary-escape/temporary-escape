#include "GuiWidgetGroup.hpp"

using namespace Engine;

GuiWidgetGroup::GuiWidgetGroup(GuiContext& ctx) :
    GuiWidgetLayout{ctx}, name{std::to_string(reinterpret_cast<uint64_t>(this))} {
}

void GuiWidgetGroup::draw() {
    if (ctx.groupBegin(name, scrollbar, border)) {
        GuiWidgetLayout::drawInternal();
        ctx.groupEnd();
    }
}

void GuiWidgetGroup::setScrollbar(const bool value) {
    scrollbar = value;
}

void GuiWidgetGroup::setBorder(const bool value) {
    border = value;
}
