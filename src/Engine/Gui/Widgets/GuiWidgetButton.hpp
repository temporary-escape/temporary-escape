#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetButton : public GuiWidget {
public:
    static const GuiStyleButton defaultStyle;
    static const GuiStyleButton successStyle;
    static const GuiStyleButton dangerStyle;
    static const GuiStyleButton infoStyle;
    static const GuiStyleButton menuStyle;

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

    void setStyle(const GuiStyleButton* value);
    const GuiStyleButton* getStyle() const {
        return style ? style : &defaultStyle;
    }

private:
    void drawInternal() override;

    std::string label;
    ImagePtr image;
    OnClickCallback onClick;
    const GuiStyleButton* style{nullptr};
};
} // namespace Engine
