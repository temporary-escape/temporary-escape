#include "GuiWidgetImageToggle.hpp"

using namespace Engine;

const GuiStyleButton GuiWidgetImageToggle::defaultStyle{
    {
        Colors::background,
        Colors::background,
        Colors::primaryBackground,
    },
    {
        Colors::white,
        Colors::primary,
        Colors::primary,
    },
    Colors::white,
};

GuiWidgetImageToggle::GuiWidgetImageToggle(GuiContext& ctx, ImagePtr image) : GuiWidget{ctx}, image{std::move(image)} {
}

void GuiWidgetImageToggle::drawInternal() {
    if (label.empty()) {
        if (ctx.imageToggle(image, toggle, *getStyle(), color) && onClick) {
            onClick(toggle);
        }
    } else {
        if (ctx.imageToggleLabel(image, toggle, *getStyle(), color, label, textAlign) && onClick) {
            onClick(toggle);
        }
    }
}

void GuiWidgetImageToggle::setImage(ImagePtr value) {
    image = std::move(value);
}

void GuiWidgetImageToggle::setLabel(std::string value) {
    label = std::move(value);
}

void GuiWidgetImageToggle::setTextAlign(const GuiTextAlign value) {
    textAlign = value;
}

void GuiWidgetImageToggle::setColor(const Color4& value) {
    color = value;
}

void GuiWidgetImageToggle::setValue(const bool value) {
    toggle = value;
}

void GuiWidgetImageToggle::setOnClick(OnClickCallback value) {
    onClick = std::move(value);
}
