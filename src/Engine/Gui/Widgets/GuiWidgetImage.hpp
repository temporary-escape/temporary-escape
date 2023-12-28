#pragma once

#include "../../Assets/Image.hpp"
#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetImage : public GuiWidget {
public:
    explicit GuiWidgetImage(GuiContext& ctx, ImagePtr image);

    void setImage(ImagePtr value);
    const ImagePtr& getImage() const {
        return image;
    }
    void setColor(const Color4& value);
    const Color4& getColor() const {
        return color;
    }

private:
    void drawInternal() override;

    ImagePtr image;
    Color4 color{1.0f};
};
} // namespace Engine
