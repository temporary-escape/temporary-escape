#pragma once

#include "GuiWidgetButtonToggle.hpp"
#include "GuiWidgetGroup.hpp"
#include "GuiWidgetRow.hpp"

namespace Engine {
class ENGINE_API GuiWidgetTabs : public GuiWidget {
public:
    explicit GuiWidgetTabs(GuiContext& ctx, float height);

    void draw() override;

    GuiWidgetGroup& addTab(const std::string& label);

private:
    float height;
    GuiWidgetRow rowButtons;
    std::vector<GuiWidgetButtonToggle*> toggles;
    std::list<GuiWidgetGroup> groups;
    int current{0};
};
} // namespace Engine
