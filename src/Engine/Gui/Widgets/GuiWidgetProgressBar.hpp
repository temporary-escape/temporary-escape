#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetProgressBar : public GuiWidget {
public:
    explicit GuiWidgetProgressBar(GuiContext& ctx);

    void setValue(float value);
    float getValue() const {
        return progress;
    }
    void setMax(float value);
    float getMax() const {
        return max;
    }
    void setColor(const Color4& value);
    const Color4& getColor() const {
        return color;
    }

private:
    void drawInternal() override;

    float progress{50.0f};
    float max{100.0f};
    Color4 color{1.0f};
};
} // namespace Engine
