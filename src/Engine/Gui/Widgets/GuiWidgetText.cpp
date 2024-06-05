#include "GuiWidgetText.hpp"
#include "../../Font/TextShaper.hpp"

using namespace Engine;

GuiWidgetText::GuiWidgetText(GuiContext& ctx, const std::string& text, float maxWidth) : GuiWidget{ctx} {
    setText(text, maxWidth);
}

void GuiWidgetText::drawInternal() {
    ctx.text(text);
}

void GuiWidgetText::setText(const std::string& value, const float maxWidth) {
    TextWrapper wrapper{ctx.getFont(), static_cast<float>(ctx.getFontSize()), maxWidth};
    wrapper.write(value);
    wrapper.flush(value.data() + value.size());
    suggestedHeight = wrapper.getHeight();
    text = wrapper.getResult();
}
