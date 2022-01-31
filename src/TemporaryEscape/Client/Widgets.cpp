#include "Widgets.hpp"

using namespace Engine;

void Widgets::loading(GuiContext& gui, const Vector2i& viewport, const std::string& message, const float progress) {
    static const Vector2 size{400.0f, 150.0f};
    const auto pos = Vector2{viewport} / 2.0f - size / 2.0f;
    const auto flags = GuiFlag::NoInput | GuiFlag::Border | GuiFlag::Title | GuiFlag::Background |
                       GuiFlag::NoScrollbar | GuiFlag::Dynamic;

    gui.window(pos, size, "Loading", flags, [&]() {
        gui.layoutDynamic(0.0f, 1);
        gui.label(message);
        gui.layoutDynamic(25.0f, 1);
        gui.progress(progress);
    });
}
