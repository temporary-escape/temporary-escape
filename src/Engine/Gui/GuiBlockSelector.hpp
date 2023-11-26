#pragma once

#include "../Assets/Block.hpp"
#include "GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiBlockSelector : public GuiWindow {
public:
    enum class Action {
        Add,
        Remove,
        Replace,
    };

    explicit GuiBlockSelector(const Config& config, AssetsManager& assetsManager);
    ~GuiBlockSelector() override = default;

    void setBlocks(const Span<BlockPtr>& blocks);
    Action getAction() const {
        return action;
    }
    BlockPtr getSelectedBlock() const {
        return selected ? selected->block : nullptr;
    }
    VoxelShape::Type getSelectedShape() const {
        return selected ? selected->shape : VoxelShape::Type::Cube;
    }
    size_t getSelectedColor() const {
        return activeColor;
    }
    BlockPtr getHoveredBlock() const {
        return hovered ? hovered->block : nullptr;
    }
    VoxelShape::Type getHoveredShape() const {
        return hovered ? hovered->shape : VoxelShape::Type::Cube;
    }
    const Vector2& getHoveredOffset() const {
        return hoveredOffset;
    }
    template <typename Fn> void setCallbackUndo(Fn&& fn) {
        callbacks.undo = std::forward<Fn>(fn);
    }
    template <typename Fn> void setCallbackRedo(Fn&& fn) {
        callbacks.redo = std::forward<Fn>(fn);
    }
    template <typename Fn> void setCallbackSave(Fn&& fn) {
        callbacks.save = std::forward<Fn>(fn);
    }
    template <typename Fn> void setCallbackLoad(Fn&& fn) {
        callbacks.load = std::forward<Fn>(fn);
    }

private:
    static constexpr float actionBarSize{96.0f};
    static constexpr size_t actionBarItems{10};

    struct Item {
        BlockPtr block{nullptr};
        std::string name;
        VoxelShape::Type shape{VoxelShape::Cube};
    };

    struct Category {
        std::string name;
        std::vector<Item> items;
    };

    struct CategoryFilter {
        std::string label;
        bool enabled;
    };

    void loadColors(const TexturePtr& asset);
    void drawLayout(Nuklear& nuklear) override;
    void drawLayoutItems(Nuklear& nuklear);
    void drawLayoutActions(Nuklear& nuklear);
    void drawLayoutArrows(Nuklear& nuklear);
    void drawLayoutTopBar(Nuklear& nuklear);
    void beforeDraw(Nuklear& nuklear, const Vector2& viewport) override;

    Category& getCategory();

    const Config& config;
    AssetsManager& assetsManager;
    std::array<Color4, 16 * 4> colors{};

    std::vector<Category> categories;
    size_t categoryIndex{0};
    size_t pageIndex{0};
    size_t activeColor{0};
    std::vector<CategoryFilter> filterChoices;
    std::string searchQuery;
    const Item* selected{nullptr};
    const Item* hovered{nullptr};
    Action action{Action::Add};
    Vector2 hoveredOffset;

    struct {
        ImagePtr arrowUp;
        ImagePtr arrowDown;
        ImagePtr undo;
        ImagePtr redo;
        ImagePtr save;
        ImagePtr load;
        ImagePtr remove;
        ImagePtr replace;
    } images;

    struct {
        std::function<void()> undo;
        std::function<void()> redo;
        std::function<void()> save;
        std::function<void()> load;
    } callbacks;
};
} // namespace Engine
