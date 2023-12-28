#pragma once

#include "GuiWidgetLayout.hpp"

namespace Engine {
class ENGINE_API GuiWidgetTemplateRow : public GuiWidgetLayout {
public:
    explicit GuiWidgetTemplateRow(GuiContext& ctx, float height);

    void draw() override;

    void setHeight(float value);
    float getHeight() const {
        return height;
    }

private:
    float height;
};
} // namespace Engine
