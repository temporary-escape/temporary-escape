#pragma once

#include "GuiWidgetLayout.hpp"

namespace Engine {
class ENGINE_API GuiWidgetGroup : public GuiWidgetLayout {
public:
    explicit GuiWidgetGroup(GuiContext& ctx);

    void draw() override;

    void setScrollbar(bool value);
    void setBorder(bool value);

private:
    std::string name;
    bool scrollbar{false};
    bool border{true};
};
} // namespace Engine
