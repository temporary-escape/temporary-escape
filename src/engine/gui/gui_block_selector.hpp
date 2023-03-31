#pragma once

#include "../assets/block.hpp"
#include "gui_window.hpp"

namespace Engine {
class ENGINE_API GuiBlockSelector : public GuiWindow {
public:
    explicit GuiBlockSelector();

    ~GuiBlockSelector() override = default;

    void setBlocks(const Span<BlockPtr>& blocks);

private:
    struct Item {
        BlockPtr block;
        std::string name;
    };

    struct Category {
        std::string name;
        std::vector<Item> items;
    };

    struct CategoryFilter {
        std::string label;
        bool enabled;
    };

    void drawLayout(Nuklear& nuklear) override;
    void drawSearchBar(Nuklear& nuklear);
    void drawCategories(Nuklear& nuklear);
    void drawBlockSelection(Nuklear& nuklear);
    void beforeDraw(Nuklear& nuklear, const Vector2& viewport) override;

    std::vector<Category> categories;
    std::vector<CategoryFilter> filterChoices;
    std::string searchQuery;
};
} // namespace Engine
