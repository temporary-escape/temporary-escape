#include "gui_block_selector.hpp"

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
        auto it = categoryIndex.find(block->getCategory());
        if (it == categoryIndex.end()) {
            it = categoryIndex.insert(std::make_pair(block->getCategory(), categories.size())).first;
            categories.emplace_back();
            categories.back().name = block->getCategory();
            filterChoices.emplace_back();
            filterChoices.back().label = block->getCategory();
            filterChoices.back().enabled = true;
        }

        categories.at(it->second).blocks.push_back(block);
    }
}

void GuiBlockSelector::drawSearchBar(Nuklear& nuklear) {
    const auto contentRegion = nuklear.getContentRegion();
    nuklear.layoutTemplateBegin(30.0f);
    nuklear.layoutTemplateStatic(contentRegion.x * 0.2f);
    nuklear.layoutTemplateDynamic();
    nuklear.layoutTemplateStatic(80.0f);
    nuklear.layoutTemplateEnd();

    nuklear.label("Filter:");
    nuklear.input(searchQuery, 128);
    if (nuklear.button("Clear")) {
        searchQuery.clear();
    }
}

void GuiBlockSelector::drawCategories(Nuklear& nuklear) {
    nuklear.layoutDynamic(0.0f, 1);
    for (auto& choice : filterChoices) {
        nuklear.select(choice.label, choice.enabled, Nuklear::TextAlign::Left);
    }
}

void GuiBlockSelector::drawBlockSelection(Nuklear& nuklear) {
    const auto width = nuklear.getContentRegion().x;
    nuklear.layoutStatic(128.0f, 128.0f, static_cast<int>(std::floor(width / 128.0f)));

    for (size_t i = 0; i < categories.size(); i++) {
        if (!filterChoices[i].enabled) {
            continue;
        }
        for (const auto& block : categories[i].blocks) {
            if (!searchQuery.empty() && block->getLabel().find(searchQuery) == std::string::npos) {
                continue;
            }
            nuklear.setDragAndDrop(block, block->getThumbnail());
            nuklear.tooltip(block->getLabel());
            nuklear.image(block->getThumbnail());
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
