#include "gui_block_action_bar.hpp"
#include "../../assets/registry.hpp"

using namespace Engine;

GuiBlockActionBar::GuiBlockActionBar(const Config& config, Preferences& preferences, Registry& registry) :
    config{config}, preferences{preferences}, registry{registry} {

    setFlags(Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Border);
    defaultImage = registry.getImages().find("block_empty_image");

    for (size_t i = 0; i < preferences.items.size(); i++) {
        if (!preferences.items[i]) {
            continue;
        }

        items[i].block = *preferences.items[i];
        items[i].image = items[i].block->getThumbnail();
    }
}

void GuiBlockActionBar::drawLayout(Nuklear& nuklear) {
    nuklear.layoutDynamic(config.gui.actionBarSize, static_cast<int>(items.size()));
    for (size_t i = 0; i < items.size(); i++) {
        auto& item = items[i];
        bool value = activeItem == i;
        if (const auto block = nuklear.getDragAndDrop<BlockPtr>()) {
            item.block = *block;
            item.image = block->get()->getThumbnail();
            preferences.items[i] = *block;
        }
        nuklear.imageToggle(item.image ? item.image : defaultImage, value);
        if (value) {
            activeItem = i;
        }
    }
}

void GuiBlockActionBar::beforeDraw(Nuklear& nuklear, const Vector2& viewport) {
    const Vector2 contentSize{
        (config.gui.actionBarSize + nuklear.getSpacing().x) * (static_cast<float>(items.size()) - 1.0f) +
            config.gui.actionBarSize,
        config.gui.actionBarSize,
    };
    setSize(nuklear.getWindowSizeForContentRegion(contentSize));
    setPos({viewport.x / 2 - getSize().x / 2, viewport.y - getSize().y});
}
