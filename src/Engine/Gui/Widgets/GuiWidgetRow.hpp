#pragma once

#include "GuiWidgetLayout.hpp"

namespace Engine {
class ENGINE_API GuiWidgetRow : public GuiWidgetLayout {
public:
    explicit GuiWidgetRow(GuiContext& ctx, float height, int columns);

    void draw() override;

    void setHeight(float value);
    float getHeight() const {
        return height;
    }
    void setColumns(int value);
    int getColumns() const {
        return columns;
    }

private:
    float height;
    int columns;
};
} // namespace Engine
