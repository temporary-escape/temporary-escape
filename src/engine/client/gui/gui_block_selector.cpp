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

void GuiBlockSelector::drawLayout(Nuklear& nuklear) {
    const auto contentRegion = nuklear.getContentRegion();
    nuklear.layoutBeginDynamic(contentRegion.y, 2);

    nuklear.layoutPush(0.2f);
    if (nuklear.groupBegin("Categories", true)) {
        nuklear.layoutDynamic(0.0f, 1);
        for (auto& choice : filterChoices) {
            nuklear.select(choice.label, choice.enabled, Nuklear::TextAlign::Left);
        }
        nuklear.groupEnd();
    }

    nuklear.layoutPush(0.8f);
    if (nuklear.groupBegin("Blocks", true)) {
        const auto width = nuklear.getContentRegion().x;
        nuklear.layoutStatic(128.0f, 128.0f, static_cast<int>(std::floor(width / 128.0f)));

        for (size_t i = 0; i < categories.size(); i++) {
            if (!filterChoices[i].enabled) {
                continue;
            }
            for (const auto& block : categories[i].blocks) {
                nuklear.image(block->getThumbnail());
            }
        }
        nuklear.groupEnd();
    }

    nuklear.layoutEnd();
}

void GuiBlockSelector::beforeDraw(const Vector2& viewport) {
}
