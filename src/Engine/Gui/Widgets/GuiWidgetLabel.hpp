#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetLabel : public GuiWidget {
public:
    explicit GuiWidgetLabel(GuiContext& ctx, std::string label);

    void draw() override;

    void setLabel(std::string value);
    const std::string& getLabel() const {
        return label;
    }

private:
    std::string label;
};
} // namespace Engine
