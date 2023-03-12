#include "gui_block_action_bar.hpp"
#include "../../assets/registry.hpp"
#include "../../math/utils.hpp"
#include "../../utils/png_importer.hpp"

using namespace Engine;

GuiBlockActionBar::GuiBlockActionBar(const Config& config, Preferences& preferences, Registry& registry,
                                     VoxelPalette& voxelPalette) :
    config{config}, preferences{preferences}, registry{registry}, voxelPalette{voxelPalette} {

    setFlags(Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Border);
    defaultImage = registry.getImages().find("block_empty_image");

    for (size_t b = 0; b < preferences.items.size(); b++) {
        for (size_t i = 0; i < preferences.items[b].size(); i++) {
            items[b][i].block = preferences.items[b][i].block;
            items[b][i].shape = preferences.items[b][i].shape;
        }
    }

    activeBar = preferences.activeBar % items.size();
    activeColor = preferences.activeColor % voxelPalette.getSize();
}

void GuiBlockActionBar::drawLayout(Nuklear& nuklear) {
    nuklear.layoutTemplateBegin(25.0f);
    for (size_t i = 0; i < items.size(); i++) {
        nuklear.layoutTemplateStatic(30.0f);
    }
    nuklear.layoutTemplateDynamic();
    nuklear.layoutTemplateStatic(100.0f);
    nuklear.layoutTemplateStatic(150.0f);
    nuklear.layoutTemplateEnd();

    for (size_t i = 0; i < items.size(); i++) {
        auto isActive = i == activeBar;
        nuklear.tooltip(fmt::format("Switch toolbar to row: {}", (i + 1) % 10));
        nuklear.buttonToggle(fmt::format("{}", (i + 1) % 10), isActive);
        if (isActive) {
            activeBar = i;
            preferences.activeBar = activeBar;
        }
    }

    nuklear.layoutSkip();
    nuklear.label("Color: ", Nuklear::TextAlign::Right);

    if (nuklear.comboBegin(voxelPalette.getColor(activeColor), {400.0f, 110.0f})) {
        nuklear.layoutDynamic(20.0f, 16);
        for (size_t i = 0; i < voxelPalette.getSize(); i++) {
            if (nuklear.button(voxelPalette.getColor(i))) {
                nuklear.comboClose();
                activeColor = i;
                preferences.activeColor = i;
            }
        }
        nuklear.comboEnd();
    }

    nuklear.layoutDynamic(config.gui.actionBarSize, static_cast<int>(items[activeBar].size()));
    for (size_t i = 0; i < items[activeBar].size(); i++) {
        auto& item = items[activeBar][i];
        bool value = activeItem == i;

        if (const auto pair = nuklear.getDragAndDrop<ActionBarBlock>()) {
            item.block = pair->block;
            item.shape = pair->shape;

            preferences.items[activeBar][i].block = item.block;
            preferences.items[activeBar][i].shape = item.shape;
        }

        if (item.block) {
            const auto tooltip =
                fmt::format("{} ({})", item.block->getLabel(), VoxelShape::typeFriendlyNames[item.shape]);
            nuklear.tooltip(tooltip);
        }

        // Clear
        if (nuklear.isClicked(MouseButton::Middle)) {
            item.block = nullptr;
            preferences.items[activeBar][i].block = nullptr;
        }

        nuklear.imageToggle(item.block ? item.block->getThumbnail(item.shape) : defaultImage, value);
        if (value) {
            activeItem = i;
        }
    }
}

void GuiBlockActionBar::beforeDraw(Nuklear& nuklear, const Vector2& viewport) {
    const Vector2 contentSize{
        (config.gui.actionBarSize + nuklear.getSpacing().x) * (static_cast<float>(items.size()) - 1.0f) +
            config.gui.actionBarSize,
        config.gui.actionBarSize + 25.0f + nuklear.getSpacing().y,
    };
    setSize(nuklear.getWindowSizeForContentRegion(contentSize));
    setPos({viewport.x / 2 - getSize().x / 2, viewport.y - getSize().y});
}
