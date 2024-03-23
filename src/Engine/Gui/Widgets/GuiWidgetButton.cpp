#include "GuiWidgetButton.hpp"

using namespace Engine;

const GuiStyleButton GuiWidgetButton::defaultStyle{
    {
        .normal = Colors::background,
        .hover = Colors::white,
        .active = Colors::primary,
    },
    {
        .normal = Colors::text,
        .hover = Colors::black,
        .active = Colors::black,
    },
};

const GuiStyleButton GuiWidgetButton::successStyle{
    {
        .normal = Colors::secondary,
        .hover = Colors::white,
        .active = Colors::primary,
    },
    {
        .normal = Colors::black,
        .hover = Colors::black,
        .active = Colors::black,
    },
};

const GuiStyleButton GuiWidgetButton::dangerStyle{
    {
        .normal = Colors::ternary,
        .hover = Colors::white,
        .active = Colors::primary,
    },
    {
        .normal = Colors::black,
        .hover = Colors::black,
        .active = Colors::black,
    },
};

GuiWidgetButton::GuiWidgetButton(GuiContext& ctx, std::string label) : GuiWidget{ctx}, label{std::move(label)} {
}

void GuiWidgetButton::drawInternal() {
    if (ctx.button(label, *getStyle(), image)) {
        if (onClick) {
            onClick();
        }
    }
}

void GuiWidgetButton::setLabel(std::string value) {
    label = std::move(value);
    ctx.setDirty();
}

void GuiWidgetButton::setImage(ImagePtr value) {
    image = std::move(value);
    ctx.setDirty();
}

void GuiWidgetButton::setOnClick(OnClickCallback value) {
    onClick = std::move(value);
}

void GuiWidgetButton::setStyle(const GuiStyleButton* value) {
    style = value;
}
