#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetProgressBar : public GuiWidget {
public:
    static const GuiStyleProgress defaultStyle;

    explicit GuiWidgetProgressBar(GuiContext& ctx);

    void setValue(float value);
    float getValue() const {
        return progress;
    }
    void setMax(float value);
    float getMax() const {
        return max;
    }
    void setHeight(float value);
    float getHeight() const {
        return height;
    }
    void setStyle(const GuiStyleProgress* value);
    const GuiStyleProgress* getStyle() const {
        return style ? style : &defaultStyle;
    }

private:
    void drawInternal() override;

    float progress{50.0f};
    float max{100.0f};
    float height{0.0f};
    const GuiStyleProgress* style{nullptr};
};
} // namespace Engine
