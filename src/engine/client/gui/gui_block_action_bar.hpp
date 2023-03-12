#pragma once

#include "../../assets/block.hpp"
#include "gui_window.hpp"

namespace Engine {
struct ENGINE_API ActionBarBlock {
    BlockPtr block{nullptr};
    VoxelShape::Type shape{VoxelShape::Cube};

    YAML_DEFINE(block, shape);
};

class ENGINE_API GuiBlockActionBar : public GuiWindow {
public:
    struct Preferences {
        std::array<std::array<ActionBarBlock, 10>, 10> items{};
        size_t activeBar{0};
        size_t activeColor{0};

        YAML_DEFINE(items, activeBar, activeColor);
    };

    explicit GuiBlockActionBar(const Config& config, Preferences& preferences, Registry& registry,
                               VoxelPalette& voxelPalette);
    ~GuiBlockActionBar() override = default;

    [[nodiscard]] size_t getActiveIndex() const {
        return activeItem;
    }

    [[nodiscard]] const ActionBarBlock& getActiveBlock() const {
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
    struct ActionBarItem : ActionBarBlock {};

    void drawLayout(Nuklear& nuklear) override;
    void beforeDraw(Nuklear& nuklear, const Vector2& viewport) override;

    const Config& config;
    Preferences& preferences;
    Registry& registry;
    VoxelPalette& voxelPalette;
    std::array<std::array<ActionBarItem, 10>, 10> items;
    size_t activeItem{0};
    size_t activeBar{0};
    size_t activeColor{0};
    ImagePtr defaultImage;
    bool colorPicker{true};
};
} // namespace Engine
