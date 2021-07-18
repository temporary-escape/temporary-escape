#include "Widgets.hpp"

using namespace Scissio;

static Vector2 center(GuiContext& gui, const Vector2& size) {
    Vector2 pos = {0.0f, 0.0f};
    const auto& viewport = gui.getViewport();

    if (viewport.x != 0 && viewport.y != 0) {
        pos = {viewport.x / 2.0f - size.x / 2.0f, viewport.y / 2.0f - size.y / 2.0f};
    }

    return pos;
}

void Widgets::modal(GuiContext& gui, const std::string& title, const std::string& message) {
    const auto size = Vector2{400.0f, 100.0f};
    const auto pos = center(gui, size);

    static const auto flags = GuiFlag::NoScrollbar | GuiFlag::Background | GuiFlag::Title | GuiFlag::Border;
    gui.window(pos, size, title, flags, [&]() {
        gui.layoutDynamic(0.0f, 1);
        gui.text(message);
    });
}

void Widgets::sidebar(GuiContext& gui, std::vector<SidebarItem>& items) {
    const auto& viewport = gui.getViewport();

    const Vector2 size{50.0f + 10.0f, (items.size() * 54.0f) + 10.0f};
    const Vector2i pos = {0.0f, (viewport.y / 2.0f) - (size.y / 2.0f)};

    static const auto flags = GuiFlag::Background | GuiFlag::Border | GuiFlag::NoScrollbar;
    gui.window(pos, size, "Sidebar", flags, [&]() {
        gui.layoutDynamic(50.0f, 1);

        auto selected = items.end();
        for (auto item = items.begin(); item != items.end(); ++item) {
            gui.tooltip(item->title);
            if (gui.buttonImage(item->icon, item->active)) {
                selected = item;
                item->active = !item->active;
                item->callback();
            }
        }

        if (selected != items.end()) {
            for (auto item = items.begin(); item != items.end(); ++item) {
                if (item == selected) {
                    continue;
                }

                item->active = false;
            }
        }
    });
}

void Widgets::blockSelector(GuiContext& gui, BlockSelectorData& data,
                            const std::function<void(const BlockDto&)>& callback) {
    const auto& viewport = gui.getViewport();

    const Vector2 size{128.0f * 2.0f + 35.0f, 620.0f};
    const Vector2 pos{100.0f, (viewport.y / 2.0f) - (size.y / 2.0f)};

    static const auto flags = GuiFlag::Background | GuiFlag::Title | GuiFlag::Border | GuiFlag::NoScrollbar;
    gui.window(pos, size, "Block Selector", flags, [&]() {
        const auto bounds = gui.getContentSize();

        gui.layoutDynamic(30.0f, 1);

        if (data.selected >= data.categories.size()) {
            data.selected = 0;
        }

        auto category = data.categories.begin() + data.selected;

        gui.combo(category->label, Vector2{300, 300}, [&]() {
            for (auto it = data.categories.begin(); it != data.categories.end(); ++it) {
                gui.layoutDynamic(0.0f, 1);
                if (gui.comboItem(it->label)) {
                    category = it;
                }
            }
        });

        data.selected = std::distance(data.categories.begin(), category);

        gui.layoutDynamic(bounds.y - 35.0f, 1);
        gui.group("Blocks", 0, [&]() {
            auto counter = 0;
            for (const auto& item : category->items) {
                auto block = item.block;

                if (!block || !item.thumbnail) {
                    continue;
                }

                if (counter % 2 == 0) {
                    gui.layoutDynamic(128.0f, 2);
                }

                gui.tooltip(400.0f, [&]() {
                    gui.layoutDynamic(0.0f, 1);
                    gui.title(block->title);
                    gui.layoutDynamic(100.0f, 1);
                    gui.text(block->description);
                });

                if (gui.buttonImage(item.thumbnail)) {
                    data.block = block;
                    callback(*block);
                }
                counter++;
            }
        });
    });
}
