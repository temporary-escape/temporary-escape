#include "Widget.hpp"

using namespace Scissio;

Widget::Widget(GuiContext& gui) : gui(gui) {
}

void Widget::render() {
    const auto flags = GuiFlag::Background | GuiFlag::Border | GuiFlag::Title;
    gui.window({50.0f, 50.0f}, {300.0f, 400.0f}, "Debug Stats", flags, [&]() { renderInternal(gui); });
}
