#include "GuiWidgetCombo.hpp"

using namespace Engine;

GuiWidgetCombo::GuiWidgetCombo(GuiContext& ctx) : GuiWidget{ctx} {
}

void GuiWidgetCombo::drawInternal() {
    Vector2 size{
        ctx.getWidgetSize().x,
        200.0f,
    };

    const auto& label = choices.empty() ? std::string{""} : choices.at(chosen);

    bool begin{false};
    if (begin = ctx.comboBegin(size, label); begin) {
        for (size_t i = 0; i < choices.size(); i++) {
            if (ctx.comboItem(choices[i])) {
                chosen = i;
                onSelected(chosen, choices[chosen]);
            }
        }
    }

    ctx.comboEnd(begin);
}

void GuiWidgetCombo::setOnSelected(OnSelectedCallback value) {
    onSelected = std::move(value);
}

void GuiWidgetCombo::addChoice(const std::string_view& value) {
    choices.emplace_back(value);
}

void GuiWidgetCombo::setChosen(const size_t value) {
    chosen = std::min(value, choices.size());
}

void GuiWidgetCombo::clear() {
    choices.clear();
    chosen = 0;
}
