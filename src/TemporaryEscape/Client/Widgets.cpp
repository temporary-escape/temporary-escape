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

            gui.onNextDropOff(item.onDrop);
            if (gui.buttonImage(item.image) && item.onClick) {
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

void Widgets::blockBrowser(Widgets::BlockBrowserData& data) {
    static const auto size = Vector2{1200.0f, 610.0f};
    const auto pos = Vector2{viewport} / 2.0f - size / 2.0f;
    static const auto flags = GuiFlag::NoScrollbar | GuiFlag::Background | GuiFlag::Border | GuiFlag::Title;
    static const auto groupFlags = GuiFlag::Scaleable;
    static const auto columnsNum = 5;

    BlockBrowserItem* hovered = nullptr;

    gui.window(pos, size, "Block Browser", flags, [&]() {
        gui.layoutDynamic(30.0f, 3, [&]() {
            gui.layoutDynamicPush(0.2f);
            gui.spacing();

            gui.layoutDynamicPush(0.7f);
            gui.input(data.filter);

            gui.layoutDynamicPush(0.1f);
            if (gui.button("Clear")) {
                data.filter = "";
            }
        });

        gui.layoutDynamic(500.0f, 3, [&]() {
            gui.layoutDynamicPush(0.2f);

            gui.group("Categories", groupFlags, [&]() {
                for (auto& category : data.categories) {
                    gui.layoutDynamic(0.0f, 1);
                    auto shown = category.shown;
                    gui.selectableLabel(category.label, shown);
                    if (shown != category.shown && shown) {
                        for (auto& other : data.categories) {
                            other.shown = &category == &other;
                        }
                    }
                }
            });

            gui.layoutDynamicPush(0.5f);
            gui.group("Blocks", groupFlags, [&]() {
                auto width = gui.getContentSize().x - GuiTheme::padding;
                auto column = std::floor(width / columnsNum - GuiTheme::padding);

                for (auto& category : data.categories) {
                    if (!category.shown) {
                        continue;
                    }

                    for (size_t i = 0; i < category.items.size(); i++) {
                        auto& item = category.items.at(i);
                        if (i % columnsNum == 0) {
                            gui.layoutDynamic(column, columnsNum);
                        }
                        if (gui.isNextHover()) {
                            hovered = &item;
                        }
                        if (gui.buttonImage(item.image)) {
                            if (item.onClick) {
                                item.onClick();
                            }
                            gui.setDragAndDrop(item.image, item.drag);
                        }
                    }

                    break;
                }
            });

            gui.layoutDynamicPush(0.3f);
            gui.group("Information", 0, [&]() {
                gui.layoutDynamic(0.0f, 1);
                gui.label("Title");
            });
        });

        static const std::vector<GuiRowTemplate> tmpl = {guiRowVariable(1.0f), guiRowStatic(100.0f)};
        gui.layoutTemplated(30.0f, tmpl);
        gui.spacing();
        if (gui.button("Close") && data.onClose) {
            data.onClose();
        }
    });

    if (hovered != data.hovered) {
        data.hovered = hovered;
        if (data.onHover) {
            data.onHover(hovered);
        }
    }
}
