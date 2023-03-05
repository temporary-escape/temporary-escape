#pragma once

#include "../../assets/block.hpp"
#include "gui_window.hpp"

namespace Engine {
class ENGINE_API GuiBlockSelector : public GuiWindow {
public:
    explicit GuiBlockSelector();

    ~GuiBlockSelector() override = default;

    void setBlocks(const Span<BlockPtr>& blocks);

private:
    struct Category {
        std::string name;
        std::vector<BlockPtr> blocks;
    };

    struct CategoryFilter {
        std::string label;
        bool enabled;
    };

    void drawLayout(Nuklear& nuklear) override;
    void beforeDraw(const Vector2& viewport) override;

    std::vector<Category> categories;
    std::vector<CategoryFilter> filterChoices;
};
} // namespace Engine
