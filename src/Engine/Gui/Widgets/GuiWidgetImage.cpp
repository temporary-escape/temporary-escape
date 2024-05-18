#include "GuiWidgetImage.hpp"

using namespace Engine;

GuiWidgetImage::GuiWidgetImage(GuiContext& ctx, ImagePtr image) : GuiWidget{ctx}, image{std::move(image)} {
}

void GuiWidgetImage::drawInternal() {
    ctx.image(image, color);
}

void GuiWidgetImage::setImage(ImagePtr value) {
    image = std::move(value);
}

void GuiWidgetImage::setColor(const Color4& value) {
    color = value;
}
