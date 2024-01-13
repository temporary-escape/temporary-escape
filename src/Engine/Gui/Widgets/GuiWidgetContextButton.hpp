#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetContextButton : public GuiWidget {
public:
    using OnClickCallback = std::function<void()>;

    explicit GuiWidgetContextButton(GuiContext& ctx, ImagePtr imageLeft, ImagePtr imageRight, std::string label);

    void setLabel(std::string value);
    const std::string& getLabel() const {
        return label;
    }
    void setImageLeft(ImagePtr value);
    const ImagePtr& getImageLeft() const {
        return imageLeft;
    }
    void setImageRight(ImagePtr value);
    const ImagePtr& getImageRight() const {
        return imageRight;
    }
    void setOnClick(OnClickCallback value);

private:
    void drawInternal() override;

    std::string label;
    ImagePtr imageLeft;
    ImagePtr imageRight;
    OnClickCallback onClick;
};
} // namespace Engine
