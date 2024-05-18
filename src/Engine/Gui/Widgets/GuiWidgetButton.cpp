#include "GuiWidgetButton.hpp"

using namespace Engine;

const GuiStyleButton GuiWidgetButton::defaultStyle{
    {
        .normal = Colors::background,
        .hover = Colors::background,
        .active = Colors::primaryBackground,
    },
    {
        .normal = Colors::white,
        .hover = Colors::primary,
        .active = Colors::primary,
    },
    Colors::white,
};

const GuiStyleButton GuiWidgetButton::successStyle{
    {
        .normal = Colors::successBackground,
        .hover = Colors::successBackground,
        .active = Colors::primaryBackground,
    },
    {
        .normal = Colors::success,
        .hover = Colors::white,
        .active = Colors::white,
    },
    Colors::success,
};

const GuiStyleButton GuiWidgetButton::dangerStyle{
    {
        .normal = Colors::dangerBackground,
        .hover = Colors::dangerBackground,
        .active = Colors::primaryBackground,
    },
    {
        .normal = Colors::danger,
        .hover = Colors::white,
        .active = Colors::white,
    },
    Colors::danger,
};

const GuiStyleButton GuiWidgetButton::menuStyle{
    {
        .normal = Colors::border,
        .hover = Colors::border,
        .active = Colors::primaryBackground,
    },
    {
        .normal = Colors::white,
        .hover = Colors::primary,
        .active = Colors::primary,
    },
    Colors::transparent,
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
}

void GuiWidgetButton::setImage(ImagePtr value) {
    image = std::move(value);
}

void GuiWidgetButton::setOnClick(OnClickCallback value) {
    onClick = std::move(value);
}

void GuiWidgetButton::setStyle(const GuiStyleButton* value) {
    style = value;
}
