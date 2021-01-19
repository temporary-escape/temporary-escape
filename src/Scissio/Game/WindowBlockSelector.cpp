#include "WindowBlockSelector.hpp"
#include "../Assets/AssetManager.hpp"

Scissio::WindowBlockSelector::WindowBlockSelector(GuiContext& ctx, AssetManager& assetManager)
    : GuiWindow(ctx, "Block Selector") {
    setSize({300, 620});
    setPos({60, 50});
    setFlags(GuiFlag::Title | GuiFlag::Background | GuiFlag::Border | GuiFlag::NoScrollbar | GuiFlag::CenterY);

    const auto blocks = assetManager.findAll<Block>();

    std::map<std::string, std::list<BlockPtr>> sorted;
    Category all{"All", {}};

    for (const auto& block : blocks) {
        sorted[block->getGroup()].emplace_back(block);
        all.choices.emplace_back(block);
    }

    categories.push_back(std::move(all));

    for (auto& pair : sorted) {
        categories.emplace_back(Category{pair.first, std::move(pair.second)});
    }

    category = categories.begin();
}

void Scissio::WindowBlockSelector::render() {
    const auto bounds = ctx.getContentSize();
    ctx.layoutDynamic(30.0f, 1);

    ctx.combo(category->label, Vector2{300, 300}, [&]() {
        for (auto it = categories.begin(); it != categories.end(); ++it) {
            ctx.layoutDynamic(0.0f, 1);
            if (ctx.comboItem(it->label.c_str())) {
                category = it;
            }
        }
    });

    ctx.layoutDynamic(bounds.y - 35.0f, 1);
    ctx.group("Blocks", 0, [&]() {
        int counter = 0;
        for (auto it = category->choices.begin(); it != category->choices.end(); ++it) {
            const auto block = *it;

            if (counter % 2 == 0) {
                ctx.layoutDynamic(128.0f, 2);
            }

            ctx.tooltip(400.0f, [&]() {
                ctx.layoutDynamic(0.0f, 1);
                ctx.title(block->getTitle());
                ctx.layoutDynamic(100.0f, 1);
                ctx.text(block->getDescription());
            });

            if (ctx.buttonImage(block->getThumbnail())) {
                Log::w("Selected: '{}'", block->getName());
            }
            counter++;
        }
    });
}
