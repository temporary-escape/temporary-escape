#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetButton : public GuiWidget {
public:
    using OnClickCallback = std::function<void()>;

    explicit GuiWidgetButton(GuiContext& ctx, std::string label);

    void setLabel(std::string value);
    const std::string& getLabel() const {
        return label;
    }
    void setImage(ImagePtr value);
    const ImagePtr& getImage() const {
        return image;
    }
    void setOnClick(OnClickCallback value);

    void setStyle(const GuiStyleButton& value);
    const GuiStyleButton& getStyle() const {
        return *style;
    }

private:
    void drawInternal() override;

    std::string label;
    ImagePtr image;
    OnClickCallback onClick;
    const GuiStyleButton* style;
};
} // namespace Engine
