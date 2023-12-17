#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetButtonToggle : public GuiWidget {
public:
    using OnClickCallback = std::function<void(bool)>;

    explicit GuiWidgetButtonToggle(GuiContext& ctx, std::string label);

    void draw() override;

    void setLabel(std::string value);
    const std::string& getLabel() const {
        return label;
    }
    void setOnClick(OnClickCallback value);
    void setValue(bool value);
    bool getValue() const {
        return toggle;
    }

private:
    std::string label;
    OnClickCallback onClick;
    bool toggle{false};
};
} // namespace Engine
