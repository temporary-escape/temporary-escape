#pragma once

#include "../Gui/GuiContext.hpp"
#include "../Utils/Log.hpp"

namespace Scissio {
class Widget {
public:
    Widget(GuiContext& gui);
    void render();

private:
    GuiContext& gui;

    virtual void renderInternal(GuiContext& gui) = 0;
};
} // namespace Scissio
