#pragma once

#include "GuiWidgetLayout.hpp"

namespace Engine {
class ENGINE_API GuiWidgetGroup : public GuiWidgetLayout {
public:
    explicit GuiWidgetGroup(GuiContext& ctx);

    void draw() override;

private:
    std::string name;
};
} // namespace Engine
