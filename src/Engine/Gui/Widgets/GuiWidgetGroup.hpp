#pragma once

#include "GuiWidgetLayout.hpp"

namespace Engine {
class ENGINE_API GuiWidgetGroup : public GuiWidgetLayout {
public:
    explicit GuiWidgetGroup(GuiContext& ctx);

    void draw() override;

    void setScrollbar(bool value);

    void setStyle(const GuiStyleGroup& value);
    const GuiStyleGroup& getStyle() const {
        return *style;
    }

private:
    std::string name;
    bool scrollbar{false};
    const GuiStyleGroup* style;
};
} // namespace Engine
