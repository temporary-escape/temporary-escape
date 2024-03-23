#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetLabel : public GuiWidget {
public:
    explicit GuiWidgetLabel(GuiContext& ctx, std::string label);

    void setLabel(std::string value);
    const std::string& getLabel() const {
        return label;
    }

    void setColor(const Color4& value);
    const Color4& getColor() const {
        return color;
    }

private:
    void drawInternal() override;

    std::string label;
    Color4 color;
};
} // namespace Engine
