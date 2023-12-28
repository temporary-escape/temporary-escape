#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetCheckbox : public GuiWidget {
public:
    using OnClickCallback = std::function<void(bool)>;

    explicit GuiWidgetCheckbox(GuiContext& ctx, std::string label);

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
    void drawInternal() override;

    std::string label;
    OnClickCallback onClick;
    bool toggle{false};
};
} // namespace Engine
