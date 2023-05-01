#pragma once

#include "../assets/block.hpp"
#include "gui_window.hpp"

namespace Engine {
class ENGINE_API GuiBlockActionBar : public GuiWindow {
public:
    struct ActionBarItem {
        BlockPtr block{nullptr};
        VoxelShape::Type shape{VoxelShape::Cube};
    };

    explicit GuiBlockActionBar(const Config& config, Registry& registry);
    ~GuiBlockActionBar() override = default;

    [[nodiscard]] size_t getActiveIndex() const {
        return activeItem;
    }

    [[nodiscard]] const ActionBarItem& getActiveBlock() const {
        return items[activeBar][activeItem];
    }

    [[nodiscard]] size_t getActiveColor() const {
        return activeColor;
    }

    void setActiveIndex(const size_t value) {
        activeItem = value;
    }

    [[nodiscard]] size_t getMaxItems() const {
        return items.size();
    }

private:
    void loadColors(const TexturePtr& asset);
    void drawLayout(Nuklear& nuklear) override;
    void beforeDraw(Nuklear& nuklear, const Vector2& viewport) override;

    const Config& config;
    Registry& registry;
    std::array<Color4, 16 * 4> colors;
    std::array<std::array<ActionBarItem, 10>, 10> items;
    size_t activeItem{0};
    size_t activeBar{0};
    size_t activeColor{0};
    ImagePtr defaultImage;
    bool colorPicker{true};
};
} // namespace Engine
