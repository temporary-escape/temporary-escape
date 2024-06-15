#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetButtonToggle : public GuiWidget {
public:
    using OnClickCallback = std::function<void(bool)>;

    explicit GuiWidgetButtonToggle(GuiContext& ctx, std::string label);

    void setLabel(std::string value);
    const std::string& getLabel() const {
        return label;
    }
    void setOnClick(OnClickCallback value);
    void setValue(bool value);
    bool getValue() const {
        return toggle;
    }
    void setStyle(const GuiStyleButton& value);
    const GuiStyleButton& getStyle() const {
        return *style;
    }

private:
    void drawInternal() override;

    std::string label;
    OnClickCallback onClick;
    bool toggle{false};
    const GuiStyleButton* style;
};
} // namespace Engine
