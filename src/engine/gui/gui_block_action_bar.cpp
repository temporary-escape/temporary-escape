#include "gui_block_action_bar.hpp"
#include "../assets/registry.hpp"
#include "../math/utils.hpp"
#include "../utils/png_importer.hpp"

using namespace Engine;

GuiBlockActionBar::GuiBlockActionBar(const Config& config, Registry& registry) : config{config}, registry{registry} {

    setFlags(Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Border);
    defaultImage = registry.getImages().find("block_empty_image");

    loadColors(registry.getTextures().find("palette"));
}

void GuiBlockActionBar::loadColors(const TexturePtr& asset) {
    PngImporter img{asset->getPath()};

    if (img.getSize().x != colors.size() || img.getSize().y != 1) {
        EXCEPTION("Palette has wrong size");
    }

    if (img.getFormat() != VK_FORMAT_R8G8B8A8_UNORM) {
        EXCEPTION("Palette has wrong pixel type, expected RGBA8");
    }

    const auto* data = img.getData();
    for (size_t i = 0; i < colors.size(); i++) {
        const auto* src = &reinterpret_cast<const uint8_t*>(data)[i * 4];
        colors[i] = fromRgbBytes(src[0], src[1], src[2], 255);
        colors[i] = glm::pow(colors[i], Vector4{2.2f});
    }
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
        }
    }

    nuklear.layoutSkip();
    nuklear.label("Color: ", Nuklear::TextAlign::Right);

    if (nuklear.comboBegin(colors[activeColor], {400.0f, 110.0f})) {
        nuklear.layoutDynamic(20.0f, 16);
        for (size_t i = 0; i < colors.size(); i++) {
            if (nuklear.button(colors[i])) {
                nuklear.comboClose();
                activeColor = i;
            }
        }
        nuklear.comboEnd();
    }

    nuklear.layoutDynamic(config.gui.actionBarSize, static_cast<int>(items[activeBar].size()));
    for (size_t i = 0; i < items[activeBar].size(); i++) {
        auto& item = items[activeBar][i];
        bool value = activeItem == i;

        if (const auto pair = nuklear.getDragAndDrop<ActionBarItem>()) {
            item.block = pair->block;
            item.shape = pair->shape;
        }

        if (item.block) {
            const auto tooltip =
                fmt::format("{} ({})", item.block->getLabel(), VoxelShape::typeFriendlyNames[item.shape]);
            nuklear.tooltip(tooltip);
        }

        // Clear
        if (nuklear.isClicked(MouseButton::Middle)) {
            item.block = nullptr;
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