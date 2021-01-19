#include "WindowBuildMenu.hpp"
#include "../Assets/AssetManager.hpp"

Scissio::WindowBuildMenu::WindowBuildMenu(GuiContext& ctx, AssetManager& assetManager)
    : GuiWindow(ctx, "Block Selector") {
    setSize({48.0f + 8.0f, 320});
    setPos({0, 200});
    setFlags(GuiFlag::Background | GuiFlag::Border | GuiFlag::NoScrollbar | GuiFlag::CenterY);

    const auto findIcon = [&](const std::string& name) { return assetManager.find<Icon>("icons-" + name); };

    items.push_back({"Select", findIcon("arrow-cursor")});
    items.push_back({"Info", findIcon("info")});
    items.push_back({"Blocks", findIcon("cube")});
    items.push_back({"Paint", findIcon("paint-roller")});
    items.push_back({"Undo", findIcon("anticlockwise-rotation")});
    items.push_back({"Redo", findIcon("clockwise-rotation")});
}

void Scissio::WindowBuildMenu::render() {
    const auto bounds = ctx.getContentSize();
    ctx.layoutDynamic(48.0f, 1);

    for (const auto& item : items) {
        ctx.tooltip(item.title);
        if (ctx.buttonImage(item.icon)) {
            Log::w("Selected: '{}'", item.title);
        }
    }
}
