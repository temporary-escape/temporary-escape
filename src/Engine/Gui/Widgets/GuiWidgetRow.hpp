#pragma once

#include "GuiWidgetLayout.hpp"

namespace Engine {
class ENGINE_API GuiWidgetRow : public GuiWidgetLayout {
public:
    explicit GuiWidgetRow(GuiContext& ctx, float height, int columns);

    void draw() override;

private:
    float height;
    int columns;
};
} // namespace Engine
