#pragma once

#include "../Assets/AssetBlock.hpp"
#include "../Gui/GuiContext.hpp"
#include "../Server/Schemas.hpp"

namespace Engine {
class ENGINE_API Widgets {
public:
    enum Align : int {
        TopLeft,
        BottomLeft,
    };

    explicit Widgets(const Config& config, GuiContext& gui) : config(config), gui(gui) {
    }

    void update(const Vector2i& viewport) {
        this->viewport = viewport;
    }

    struct ContextMenuItem {
        AssetImagePtr image;
        std::string label;
        std::function<void()> onClick;
    };

    struct ActionBarItem {
        AssetImagePtr image;
        std::function<void()> onClick;
        std::string tooltip;
    };

    struct BlockBrowserItem {
        AssetImagePtr image;
        std::string label;
        std::function<void()> onHover;
    };

    void loading(const std::string& message, float progress);
    void contextMenu(const Vector2i& pos, const std::vector<ContextMenuItem>& items);
    void actionBar(float itemWidth, const std::vector<ActionBarItem>& items);
    void blockTooltip(const Vector2i& pos, const AssetBlockPtr& block, Shape::Type shapeType, Align align);
    void blockBrowser(const Vector2i& pos, const std::vector<BlockBrowserItem>& items, std::string& filter,
                      const std::function<void()>& onClose);

private:
    const Config& config;
    GuiContext& gui;
    Vector2i viewport;
};
} // namespace Engine
