#pragma once

#include "gui_window.hpp"

namespace Engine {
class ENGINE_API GuiMainMenu : public GuiWindow {
public:
    struct Item {
        std::string label;
        std::function<void()> callback;
    };

    explicit GuiMainMenu();
    ~GuiMainMenu() override = default;

    void setItems(std::vector<Item> values) {
        items = std::move(values);
    }

private:
    void drawLayout(Nuklear& nuklear) override;
    void beforeDraw(const Vector2& viewport) override;

    std::vector<Item> items;
};
} // namespace Engine
