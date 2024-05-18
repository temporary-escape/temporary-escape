#pragma once

#include "../../Assets/Image.hpp"
#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetImageToggle : public GuiWidget {
public:
    static const GuiStyleButton defaultStyle;

    using OnClickCallback = std::function<void(bool)>;

    explicit GuiWidgetImageToggle(GuiContext& ctx, ImagePtr image);

    void setImage(ImagePtr value);
    const ImagePtr& getImage() const {
        return image;
    }
    void setLabel(std::string value);
    const std::string& getLabel() const {
        return label;
    }
    void setTextAlign(GuiTextAlign value);
    GuiTextAlign getTextAlign() const {
        return textAlign;
    }
    void setColor(const Color4& value);
    const Color4& getColor() const {
        return color;
    }
    void setValue(bool value);
    bool getValue() const {
        return toggle;
    }
    void setOnClick(OnClickCallback value);

    void setStyle(const GuiStyleButton* value);
    const GuiStyleButton* getStyle() const {
        return style ? style : &defaultStyle;
    }

private:
    void drawInternal() override;

    ImagePtr image;
    std::string label;
    GuiTextAlign textAlign{GuiTextAlign::Center};
    Color4 color{1.0f};
    bool toggle{false};
    OnClickCallback onClick;
    const GuiStyleButton* style{nullptr};
};
} // namespace Engine
