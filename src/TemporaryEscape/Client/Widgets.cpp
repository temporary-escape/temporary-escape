#include "Widgets.hpp"

using namespace Engine;

void Widgets::loading(const std::string& message, const float progress) {
    static const Vector2 size{400.0f, 150.0f};
    const auto pos = Vector2{viewport} / 2.0f - size / 2.0f;
    const auto flags = GuiFlag::NoInput | GuiFlag::Border | GuiFlag::Title | GuiFlag::Background |
                       GuiFlag::NoScrollbar | GuiFlag::Dynamic;

    gui.window(pos, size, "Loading", flags, [&]() {
        gui.layoutDynamic(0.0f, 1);
        gui.label(message);
        gui.layoutDynamic(25.0f, 1);
        gui.progress(progress);
    });
}

void Widgets::contextMenu(const Vector2i& pos, const std::vector<ContextMenuItem>& items) {
    const auto flags = GuiFlag::NoScrollbar | GuiFlag::Dynamic;
    gui.window(pos, Vector2{200.0f, 400.0f}, "ContextMenu", flags, [&]() {
        gui.layoutDynamic(0.0f, 1);
        for (const auto& item : items) {
            if (gui.buttonImage(item.image, item.label) && item.onClick) {
                item.onClick();
            }
        }
    });
}

void Widgets::actionBar(const float itemWidth, const std::vector<ActionBarItem>& items) {
    const auto flags = GuiFlag::NoScrollbar | GuiFlag::Background | GuiFlag::Border;

    const auto size = Vector2{items.size() * (itemWidth + GuiTheme::padding + 2.0f) + GuiTheme::padding + 2.0f,
                              itemWidth + GuiTheme::padding * 2.0f};
    const auto pos = Vector2{viewport.x / 2.0f - size.x / 2.0f, viewport.y - size.y};

    gui.window(pos, size, "Action Bar", flags, [&]() {
        gui.layoutDynamic(itemWidth, items.size());

        for (const auto& item : items) {

            if (!item.tooltip.empty()) {
                gui.tooltip(item.tooltip);
            }

            if (gui.buttonImage(item.image, "") && item.onClick) {
                item.onClick();
            }
        }
    });
}

void Widgets::blockTooltip(const Vector2i& pos, const AssetBlockPtr& block, const Shape::Type shapeType, Align align) {
    const auto flags = GuiFlag::NoScrollbar | GuiFlag::Background | GuiFlag::Border;

    const auto textHeight = 200.0f;
    const auto size = Vector2{config.thumbnailSize + GuiTheme::padding * 2,
                              config.thumbnailSize + GuiTheme::padding * 2 + textHeight};

    gui.window(pos, size, "Block Tooltip", flags, [&]() {
        gui.layoutDynamic(config.thumbnailSize, 1);
        gui.image(block->getImageForShape(shapeType));
        gui.layoutDynamic(0.0f, 1);
        gui.label(fmt::format("{} ({})", block->getTitle(), shapeTypeToFriendlyName(shapeType)));
    });
}

void Widgets::blockBrowser(const Vector2i& pos, const std::vector<BlockBrowserItem>& items, std::string& filter,
                           const std::function<void()>& onClose) {
    const auto flags = GuiFlag::NoScrollbar | GuiFlag::Background | GuiFlag::Border | GuiFlag::Title;
    const auto groupFlags = GuiFlag::Border;

    const auto size = Vector2i{3 * (128.0f + GuiTheme::padding + 2.0f) + 20.0f, 600.0f};

    gui.window(pos, size, "Block Browser", flags, [&]() {
        gui.layoutDynamic(30.0f, 1);
        gui.input(filter);

        gui.layoutDynamic(500.0f, 1);
        gui.group("Blocks", groupFlags, [&]() {
            for (size_t i = 0; i < items.size(); i++) {
                if (i % 3 == 0) {
                    gui.layoutDynamic(128.0f, 3);
                }
                gui.buttonImage(items.at(i).image, "");
            }
        });

        gui.layoutDynamic(30.0f, 3);
        gui.spacing();
        gui.spacing();
        if (gui.button("Close") && onClose) {
            onClose();
        }
    });
}
