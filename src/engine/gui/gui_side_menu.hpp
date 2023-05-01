#pragma once

#include "../assets/block.hpp"
#include "gui_window.hpp"

namespace Engine {
class ENGINE_API GuiSideMenu : public GuiWindow {
public:
    struct Item {
        ImagePtr image;
        std::string label;
        std::function<void(bool)> callback;
        bool toggle{false};
        bool active{false};
    };

    explicit GuiSideMenu(const Config& config);
    ~GuiSideMenu() override = default;

    void setItems(std::vector<Item> value) {
        items = std::move(value);
    }

private:
    void drawLayout(Nuklear& nuklear) override;
    void drawSearchBar(Nuklear& nuklear);
    void drawCategories(Nuklear& nuklear);
    void drawBlockSelection(Nuklear& nuklear);
    void beforeDraw(Nuklear& nuklear, const Vector2& viewport) override;

    const Config& config;
    std::vector<Item> items;
};
} // namespace Engine
