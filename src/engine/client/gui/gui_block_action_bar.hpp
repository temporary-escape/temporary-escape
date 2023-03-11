#pragma once

#include "../../assets/block.hpp"
#include "gui_window.hpp"

namespace Engine {
class ENGINE_API GuiBlockActionBar : public GuiWindow {
public:
    struct Preferences {
        std::array<std::optional<BlockPtr>, 10> items{};

        YAML_DEFINE(items);
    };

    explicit GuiBlockActionBar(const Config& config, Preferences& preferences, Registry& registry);
    ~GuiBlockActionBar() override = default;

    [[nodiscard]] size_t getActiveIndex() const {
        return activeItem;
    }

    [[nodiscard]] const BlockPtr& getActiveBlock() const {
        return items[activeItem].block;
    }

    void setActiveIndex(const size_t value) {
        activeItem = value;
    }

    [[nodiscard]] size_t getMaxItems() const {
        return items.size();
    }

private:
    struct ActionBarItem {
        BlockPtr block;
        ImagePtr image;
    };

    void drawLayout(Nuklear& nuklear) override;
    void beforeDraw(Nuklear& nuklear, const Vector2& viewport) override;

    const Config& config;
    Preferences& preferences;
    Registry& registry;
    std::array<ActionBarItem, 10> items;
    size_t activeItem{0};
    ImagePtr defaultImage;
};
} // namespace Engine
