#include "Widgets.hpp"

using namespace Engine;

void Widgets::loading(const std::string& message, const float progress) {
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

void Widgets::contextMenu(const Vector2i& pos, const std::vector<ContextMenuItem>& items) {
    const auto flags = GuiFlag::NoScrollbar | GuiFlag::Dynamic;
    gui.window(pos, Vector2{200.0f, 400.0f}, "ContextMenu", flags, [&]() {
        gui.layoutDynamic(0.0f, 1);
        for (const auto& item : items) {
            if (gui.buttonImage(item.image, item.label)) {
                item.callback();
            }
        }
    });
}
