#pragma once

#include "../Assets/AssetImage.hpp"
#include "../Gui/GuiContext.hpp"
#include "../Server/Schemas.hpp"
#include "View.hpp"
#include "Widgets.hpp"

namespace Engine {
class ENGINE_API ViewBuild : public View {
public:
    explicit ViewBuild(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                       Client& client, Widgets& widgets);

    void load();
    void render(const Vector2i& viewport) override;
    void renderGui(const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;

private:
    void actionPlaceBlock();
    void fetchUnlockedBlocks();
    void setActionBarItem(size_t index, const AssetBlockPtr& block, Shape::Type shape);
    void setActionBarActive(size_t index);
    void setActionBarActiveItem(const AssetBlockPtr& block, Shape::Type shape) {
        setActionBarItem(actionBar.active, block, shape);
    }

    const Config& config;
    Canvas2D& canvas;
    AssetManager& assetManager;
    Renderer& renderer;
    Client& client;
    Widgets& widgets;

    std::unique_ptr<Scene> scene;
    EntityPtr entityShip;
    EntityPtr entityHelperAdd;
    bool loading;

    std::chrono::time_point<std::chrono::steady_clock> lastTimePoint;
    Vector2i mousePos;
    std::optional<Grid::RayCastResult> rayCastResult;

    struct {
        AssetImagePtr noThumbnail;
    } images;

    struct {
        std::array<AssetBlockPtr, 10> blocks;
        std::array<Shape::Type, 10> shapes;
        size_t active = 0;
        std::vector<Widgets::ActionBarItem> items;
    } actionBar;

    struct {
        bool enabled = false;
        std::vector<std::tuple<AssetBlockPtr, Shape::Type>> available;
        Widgets::BlockBrowserData data;
        AssetBlockPtr hovered = nullptr;
        AssetBlockPtr dragging = nullptr;
    } blockBrowser;
};
} // namespace Engine
