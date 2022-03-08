#pragma once

#include "../Gui/GuiContext.hpp"
#include "../Server/Schemas.hpp"

namespace Engine {
class ENGINE_API Widgets {
public:
    explicit Widgets(GuiContext& gui) : gui(gui) {
    }

    void update(const Vector2i& viewport) {
        this->viewport = viewport;
    }

    struct ContextMenuItem {
        AssetImagePtr image;
        std::string label;
        std::function<void()> callback;
    };

    void loading(const std::string& message, float progress);
    void contextMenu(const Vector2i& pos, const std::vector<ContextMenuItem>& items);

private:
    GuiContext& gui;
    Vector2i viewport;
};
} // namespace Engine
