#include "gui_side_menu.hpp"
#include "../assets/registry.hpp"

using namespace Engine;

GuiSideMenu::GuiSideMenu(const Config& config) : config{config} {
    setFlags(Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Border);
}

void GuiSideMenu::drawLayout(Nuklear& nuklear) {
    for (auto& item : items) {
        nuklear.layoutDynamic(config.gui.sideMenuSize, 1);

        nuklear.tooltip(item.label);

        if (item.toggle) {
            const auto previous = item.active;
            nuklear.imageToggle(item.image, item.active);
            if (previous != item.active && item.callback) {
                item.callback(item.active);
            }
        } else {
            if (nuklear.image(item.image) && item.callback) {
                item.callback(true);
            }
        }
    }
}

void GuiSideMenu::beforeDraw(Nuklear& nuklear, const Vector2& viewport) {
    const Vector2 contentSize{
        config.gui.sideMenuSize + nuklear.getSpacing().x,
        (config.gui.sideMenuSize + nuklear.getSpacing().y) * (static_cast<float>(items.size()) - 1.0f) +
            config.gui.sideMenuSize,
    };
    setSize(nuklear.getWindowSizeForContentRegion(contentSize));
    setPos({0, viewport.y / 2.0f - getSize().y / 2.0f});
}
