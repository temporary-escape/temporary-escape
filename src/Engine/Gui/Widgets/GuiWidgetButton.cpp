#include "GuiWidgetButton.hpp"

using namespace Engine;

const GuiStyleButton GuiWidgetButton::defaultStyle{
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

const GuiStyleButton GuiWidgetButton::successStyle{
    {
        Colors::successBackground,
        Colors::successBackground,
        Colors::primaryBackground,
    },
    {
        Colors::success,
        Colors::white,
        Colors::white,
    },
    Colors::success,
};

const GuiStyleButton GuiWidgetButton::dangerStyle{
    {
        Colors::dangerBackground,
        Colors::dangerBackground,
        Colors::primaryBackground,
    },
    {
        Colors::danger,
        Colors::white,
        Colors::white,
    },
    Colors::danger,
};

const GuiStyleButton GuiWidgetButton::infoStyle{
    {
        Colors::infoBackground,
        Colors::infoBackground,
        Colors::primaryBackground,
    },
    {
        Colors::info,
        Colors::white,
        Colors::white,
    },
    Colors::info,
};

const GuiStyleButton GuiWidgetButton::menuStyle{
    {
        Colors::border,
        Colors::border,
        Colors::primaryBackground,
    },
    {
        Colors::white,
        Colors::primary,
        Colors::primary,
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
