#include "gui_block_selector.hpp"
#include "../assets/assets_manager.hpp"
#include "../file/ktx2_file_reader.hpp"
#include "../math/utils.hpp"
#include "../utils/string_utils.hpp"

using namespace Engine;

GuiBlockSelector::GuiBlockSelector(const Config& config, AssetsManager& assetsManager) :
    config{config}, assetsManager{assetsManager} {

    setFlags(Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Border);

    loadColors(assetsManager.getTextures().find("palette"));

    images.arrowUp = assetsManager.getImages().find("icon_small_arrow_up");
    images.arrowDown = assetsManager.getImages().find("icon_small_arrow_down");
    images.undo = assetsManager.getImages().find("icon_anticlockwise_rotation");
    images.redo = assetsManager.getImages().find("icon_clockwise_rotation");
    images.remove = assetsManager.getImages().find("icon_remove");
    images.replace = assetsManager.getImages().find("icon_swap");
    images.save = assetsManager.getImages().find("icon_save");
    images.load = assetsManager.getImages().find("icon_open_folder");
}

void GuiBlockSelector::loadColors(const TexturePtr& asset) {
    Ktx2FileReader image{asset->getPath()};

    if (image.needsTranscoding()) {
        image.transcode(VulkanCompressionType::None, Ktx2CompressionTarget::RGBA);
    }

    image.readData();

    if (image.getFormat() != VK_FORMAT_R8G8B8A8_UNORM) {
        EXCEPTION("Image must be of format RGBA 8bit");
    }

    if (image.getSize().x != colors.size() || image.getSize().y != 1) {
        EXCEPTION("Palette has wrong size, expected {}x1", colors.size());
    }

    if (image.getFormat() != VK_FORMAT_R8G8B8A8_UNORM) {
        EXCEPTION("Palette has wrong pixel type, expected RGBA8");
    }

    const auto* data = image.getData(0, 0).pixels.data();
    for (size_t i = 0; i < colors.size(); i++) {
        const auto* src = &reinterpret_cast<const uint8_t*>(data)[i * 4];
        colors[i] = fromRgbBytes(src[0], src[1], src[2], 255);
        colors[i] = glm::pow(colors[i], Vector4{2.2f});
    }
}

void GuiBlockSelector::setBlocks(const Span<BlockPtr>& blocks) {
    categories.clear();

    std::unordered_map<std::string, size_t> indexes;

    // All
    categories.emplace_back();
    categories.back().name = "All";

    for (const auto& block : blocks) {
        auto it = indexes.find(block->getCategory());
        if (it == indexes.end()) {
            it = indexes.insert(std::make_pair(block->getCategory(), categories.size())).first;
            categories.emplace_back();
            categories.back().name = block->getCategory();
            /*filterChoices.emplace_back();
            filterChoices.back().label = block->getCategory();
            filterChoices.back().enabled = true;*/
        }

        auto& category = categories.at(it->second);
        auto& all = categories.front();

        for (const auto shape : block->getShapes()) {
            { // Block specific category
                category.items.emplace_back();
                auto& item = category.items.back();
                item.block = block;
                item.name = toLower(block->getLabel());
                item.shape = shape;
            }

            { // "All" category
                all.items.emplace_back();
                auto& item = all.items.back();
                item.block = block;
                item.name = toLower(block->getLabel());
                item.shape = shape;
            }
        }
    }
}

void GuiBlockSelector::drawLayout(Nuklear& nuklear) {
    nuklear.layoutDynamic(30.0f, 1);
    drawLayoutTopBar(nuklear);

    nuklear.layoutTemplateBegin(nuklear.getContentRegion().y);

    const auto width = actionBarSize * actionBarItems + nuklear.getSpacing().x * (actionBarItems - 1);
    nuklear.layoutTemplateStatic(width);
    drawLayoutItems(nuklear);

    nuklear.layoutTemplateStatic(32.0f);
    drawLayoutArrows(nuklear);

    nuklear.layoutTemplateStatic(64.0f * 3 + nuklear.getSpacing().x * 2.0f);
    drawLayoutActions(nuklear);

    nuklear.layoutTemplateEnd();
}

GuiBlockSelector::Category& GuiBlockSelector::getCategory() {
    if (categoryIndex >= categories.size()) {
        EXCEPTION("Category index out of range");
    }
    return categories.at(categoryIndex);
}

void GuiBlockSelector::drawLayoutItems(Nuklear& nuklear) {
    if (!nuklear.groupBegin("Blocks", false)) {
        return;
    }

    nuklear.layoutTemplateBegin(actionBarSize);

    const auto& category = getCategory();
    auto offset = category.items.begin() + static_cast<int64_t>(pageIndex * actionBarItems);

    hovered = nullptr;

    for (size_t i = 0; i < actionBarItems; i++) {
        nuklear.layoutTemplateStatic(actionBarSize);

        if (offset < category.items.end()) {
            const auto& item = *(offset++);
            auto value = selected == &item;

            if (nuklear.isHovered()) {
                hovered = &item;
                hoveredOffset = {i * (actionBarSize + nuklear.getSpacing().x), 0.0f};
            }

            nuklear.imageToggle(item.block->getThumbnail(item.shape), value);
            if (value) {
                selected = &item;
                action = Action::Add;
            }
        } else {
            nuklear.layoutSkip();
        }
    }

    nuklear.layoutTemplateEnd();

    nuklear.layoutTemplateBegin(30.0f);
    for (size_t i = 0; i < categories.size(); i++) {
        nuklear.layoutTemplateStatic(125.0f);
        auto value = categoryIndex == i;
        nuklear.buttonToggle(categories.at(i).name, value);
        if (value) {
            if (categoryIndex != i) {
                selected = nullptr;
                pageIndex = 0;
            }
            categoryIndex = i;
        }
    }
    nuklear.layoutTemplateEnd();

    nuklear.groupEnd();
}

void GuiBlockSelector::drawLayoutArrows(Nuklear& nuklear) {
    if (!nuklear.groupBegin("Arrows", false)) {
        return;
    }

    nuklear.layoutDynamic(32.0f, 1);

    nuklear.tooltip("Previous row");
    if (nuklear.buttonImage(images.arrowUp)) {
        // Void
        if (pageIndex > 0) {
            --pageIndex;
        }
    }

    nuklear.layoutDynamic(actionBarSize - 32.0f * 2.0f - nuklear.getSpacing().y * 2.0f, 1);
    nuklear.layoutSkip();

    nuklear.layoutDynamic(32.0f, 1);

    nuklear.tooltip("Next row");
    if (nuklear.buttonImage(images.arrowDown)) {
        // Void
        if (pageIndex < categories.at(categoryIndex).items.size() / actionBarItems) {
            ++pageIndex;
        }
    }

    nuklear.groupEnd();
}

void GuiBlockSelector::drawLayoutActions(Nuklear& nuklear) {
    if (!nuklear.groupBegin("Actions", false)) {
        return;
    }

    nuklear.layoutDynamic(64.0f, 3);

    nuklear.tooltip("Undo");
    if (nuklear.buttonImage(images.undo) && callbacks.undo) {
        callbacks.undo();
    }

    nuklear.tooltip("Redo");
    if (nuklear.buttonImage(images.redo) && callbacks.redo) {
        callbacks.redo();
    }

    nuklear.tooltip("Save current design");
    if (nuklear.buttonImage(images.save) && callbacks.save) {
        callbacks.save();
    }
    nuklear.layoutDynamic(64.0f, 3);

    nuklear.tooltip("Remove blocks");
    auto isRemoving = action == Action::Remove;
    nuklear.buttonImageToggle(images.remove, isRemoving);
    if (isRemoving) {
        action = Action::Remove;
        selected = nullptr;
    }

    nuklear.tooltip("Replace blocks");
    auto isReplacing = action == Action::Replace;
    nuklear.buttonImageToggle(images.replace, isReplacing);
    if (isReplacing) {
        action = Action::Replace;
    }

    nuklear.tooltip("Load design");
    if (nuklear.buttonImage(images.load) && callbacks.load) {
        callbacks.load();
    }

    nuklear.groupEnd();
}

void GuiBlockSelector::drawLayoutTopBar(Nuklear& nuklear) {
    if (!nuklear.groupBegin("Top Bar", false)) {
        return;
    }

    const auto pages = categories.at(categoryIndex).items.size() / actionBarItems + 1;

    nuklear.layoutTemplateBegin(25.0f);
    for (size_t i = 0; i < pages; i++) {
        nuklear.layoutTemplateStatic(25.0f);
    }
    nuklear.layoutTemplateDynamic();
    nuklear.layoutTemplateStatic(200.0f);
    nuklear.layoutTemplateStatic(64.0f * 3.0f + nuklear.getSpacing().x * 2.0f);
    nuklear.layoutTemplateEnd();

    for (size_t i = 0; i < pages; i++) {
        auto isActive = i == pageIndex;
        nuklear.tooltip(fmt::format("Switch to row: {}", (i + 1) % 10));
        nuklear.buttonToggle(fmt::format("{}", (i + 1) % 10), isActive);
        if (isActive) {
            pageIndex = i;
        }
    }

    nuklear.layoutSkip();

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

    nuklear.layoutSkip();

    nuklear.groupEnd();
}

void GuiBlockSelector::beforeDraw(Nuklear& nuklear, const Vector2& viewport) {
    Vector2 contentSize{nuklear.getSpacing().x, nuklear.getSpacing().y};

    // Top row
    contentSize.y += 25.0f + nuklear.getSpacing().y;
    // Blocks
    contentSize.y += actionBarSize * 1.0f + nuklear.getSpacing().y;
    // Categories
    contentSize.y += 30.0f + nuklear.getSpacing().y;

    // Blocks
    contentSize.x += actionBarSize * actionBarItems + nuklear.getSpacing().x * actionBarItems;
    // Up and down buttons
    contentSize.x += 32.0f + nuklear.getSpacing().x;
    // Action buttons
    contentSize.x += 64.0f * 3.0f + nuklear.getSpacing().x * 2.0f;

    setSize(nuklear.getWindowSizeForContentRegion(contentSize));
    setPos({viewport.x / 2 - getSize().x / 2, viewport.y - getSize().y});
}
