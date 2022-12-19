#pragma once

#include "../assets/block.hpp"
#include "gui_window.hpp"

namespace Engine {
class ENGINE_API GuiContextMenu : public GuiWindow {
public:
    struct Item {
        std::string label;
        std::function<void()> callback;
    };

    explicit GuiContextMenu();
    ~GuiContextMenu() override = default;

    void setItems(std::vector<Item> value) {
        items = std::move(value);
    }

private:
    void drawLayout(Nuklear& nuklear) override;

    std::vector<Item> items;
};
} // namespace Engine
