#include "GuiWidgetTextInput.hpp"

using namespace Engine;

GuiWidgetTextInput::GuiWidgetTextInput(GuiContext& ctx, size_t max) : GuiWidget{ctx}, max{max} {
}

void GuiWidgetTextInput::drawInternal() {
    if (ctx.textInput(input, max) && onModifyCallback) {
        onModifyCallback();
    }
}

void GuiWidgetTextInput::setValue(std::string value) {
    input = std::move(value);
}

void GuiWidgetTextInput::setOnModify(GuiWidgetTextInput::OnModifyCallback value) {
    onModifyCallback = std::move(value);
}
