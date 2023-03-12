#include "gui_block_selector.hpp"
#include "../../utils/string_utils.hpp"
#include "gui_block_action_bar.hpp"

using namespace Engine;

GuiBlockSelector::GuiBlockSelector() {
    setSize({800.0f, 500.0f});
    setPos({100.0f, 100.0f});
    setFlags(Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Border | Nuklear::WindowFlags::Title |
             Nuklear::WindowFlags::Moveable | Nuklear::WindowFlags::Closeable);
    setTitle("Block Browser");
}

void GuiBlockSelector::setBlocks(const Span<BlockPtr>& blocks) {
    categories.clear();

    std::unordered_map<std::string, size_t> categoryIndex;

    for (const auto& block : blocks) {
        /*if (block->getCategory() == "Internal") {
            continue;
        }*/

        auto it = categoryIndex.find(block->getCategory());
        if (it == categoryIndex.end()) {
            it = categoryIndex.insert(std::make_pair(block->getCategory(), categories.size())).first;
            categories.emplace_back();
            categories.back().name = block->getCategory();
            filterChoices.emplace_back();
            filterChoices.back().label = block->getCategory();
            filterChoices.back().enabled = true;
        }

        auto& category = categories.at(it->second);
        category.items.emplace_back();

        auto& item = category.items.back();
        item.block = block;
        item.name = toLower(block->getLabel());
    }
}

void GuiBlockSelector::drawSearchBar(Nuklear& nuklear) {
    const auto contentRegion = nuklear.getContentRegion();
    nuklear.layoutTemplateBegin(30.0f);
    nuklear.layoutTemplateStatic(contentRegion.x * 0.2f);
    nuklear.layoutTemplateDynamic();
    nuklear.layoutTemplateStatic(80.0f);
    nuklear.layoutTemplateEnd();

    nuklear.label("Categories:");
    nuklear.input(searchQuery, 128);
    if (nuklear.button("Clear")) {
        searchQuery.clear();
    }
}

void GuiBlockSelector::drawCategories(Nuklear& nuklear) {
    nuklear.layoutDynamic(0.0f, 1);
    for (auto& choice : filterChoices) {
        nuklear.checkbox(choice.label, choice.enabled);
    }
}

void GuiBlockSelector::drawBlockSelection(Nuklear& nuklear) {
    const auto width = nuklear.getContentRegion().x;
    nuklear.layoutStatic(128.0f, 128.0f, static_cast<int>(std::floor(width / 128.0f)));

    for (size_t i = 0; i < categories.size(); i++) {
        if (!filterChoices[i].enabled) {
            continue;
        }
        for (const auto& item : categories[i].items) {
            if (!searchQuery.empty() && item.name.find(searchQuery) == std::string::npos) {
                continue;
            }

            for (const auto shape : item.block->getShapes()) {
                const auto tooltip =
                    fmt::format("{} ({})", item.block->getLabel(), VoxelShape::typeFriendlyNames[shape]);

                nuklear.setDragAndDrop(ActionBarBlock{item.block, shape}, item.block->getThumbnail(shape));
                nuklear.tooltip(tooltip);
                nuklear.image(item.block->getThumbnail(shape));
            }
        }
    }
}

void GuiBlockSelector::drawLayout(Nuklear& nuklear) {
    drawSearchBar(nuklear);

    const auto contentRegion = nuklear.getContentRegion();
    nuklear.layoutBeginDynamic(contentRegion.y, 2);

    nuklear.layoutPush(0.2f);
    if (nuklear.groupBegin("Categories", true)) {
        drawCategories(nuklear);
        nuklear.groupEnd();
    }

    nuklear.layoutPush(0.8f);
    if (nuklear.groupBegin("Blocks", true)) {
        drawBlockSelection(nuklear);
        nuklear.groupEnd();
    }

    nuklear.layoutEnd();
}

void GuiBlockSelector::beforeDraw(Nuklear& nuklear, const Vector2& viewport) {
}
