#include "GuiWidgetLayout.hpp"

using namespace Engine;

GuiWidgetLayout::GuiWidgetLayout(GuiContext& ctx) : GuiWidget{ctx} {
}

void GuiWidgetLayout::draw() {
    for (auto& widget : widgets) {
        widget->draw();
    }
}

void GuiWidgetLayout::clearWidgets() {
    widgets.clear();
}

void GuiWidgetLayout::removeWidget(GuiWidget& widget) {
    widgets.erase(std::remove_if(widgets.begin(),
                                 widgets.end(),
                                 [&](const std::shared_ptr<GuiWidget>& w) { return w.get() == &widget; }),
                  widgets.end());
}
