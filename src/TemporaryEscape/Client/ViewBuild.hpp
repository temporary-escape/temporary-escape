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
    void renderActionBar();
    void renderBlockBrowser();
    void fetchUnlockedBlocks();

    const Config& config;
    Canvas2D& canvas;
    AssetManager& assetManager;
    Renderer& renderer;
    Client& client;
    Widgets& widgets;

    std::unique_ptr<Scene> scene;
    bool loading;

    std::chrono::time_point<std::chrono::steady_clock> lastTimePoint;

    struct {
        AssetImagePtr noThumbnail;
    } images;

    struct {
        std::array<AssetBlockPtr, 10> blocks;
        std::array<Shape::Type, 10> shapes;
        size_t active = 0;
    } actionBar;

    struct {
        bool enabled = false;
        std::vector<std::tuple<AssetBlockPtr, Shape::Type>> available;
        std::string filter;
    } blockBrowser;
};
} // namespace Engine
