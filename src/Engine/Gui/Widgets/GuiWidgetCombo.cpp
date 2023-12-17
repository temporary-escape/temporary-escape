#include "GuiWidgetCombo.hpp"

using namespace Engine;

GuiWidgetCombo::GuiWidgetCombo(GuiContext& ctx) : GuiWidget{ctx} {
}

void GuiWidgetCombo::draw() {
    Vector2 size{
        ctx.getWidgetSize().x,
        200.0f,
    };

    const auto& label = choices.empty() ? std::string{""} : choices.at(chosen);

    if (ctx.comboBegin(size, label)) {
        for (size_t i = 0; i < choices.size(); i++) {
            if (ctx.comboItem(choices[i])) {
                chosen = i;
            }
        }

        ctx.comboEnd();
    }
}

void GuiWidgetCombo::setOnSelected(OnSelectedCallback value) {
    onClick = std::move(value);
}

void GuiWidgetCombo::addChoice(const std::string_view& value) {
    choices.emplace_back(value);
}

void GuiWidgetCombo::setChosen(const size_t value) {
    chosen = std::min(value, choices.size());
}
