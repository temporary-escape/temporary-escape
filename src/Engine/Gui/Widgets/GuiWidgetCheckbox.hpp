#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetCheckbox : public GuiWidget {
public:
    using OnClickCallback = std::function<void(bool)>;

    explicit GuiWidgetCheckbox(GuiContext& ctx, std::string label, bool& value);

    void setLabel(std::string value);
    const std::string& getLabel() const {
        return label;
    }
    void setOnClick(OnClickCallback value);

private:
    void drawInternal() override;

    std::string label;
    bool* value;
    OnClickCallback onClick;
};
} // namespace Engine
