#pragma once

#include "../Assets/AssetImage.hpp"
#include "../Gui/GuiContext.hpp"
#include "../Server/Messages.hpp"
#include "../Server/Schemas.hpp"
#include "View.hpp"
#include "Widgets.hpp"

namespace Engine {
class ENGINE_API ViewSpace : public View {
public:
    explicit ViewSpace(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                       Client& client, Widgets& widgets);

    void render(const Vector2i& viewport) override;
    void renderGui(const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;

private:
    const Config& config;
    Canvas2D& canvas;
    AssetManager& assetManager;
    Renderer& renderer;
    Client& client;
    Widgets& widgets;

    AssetFontFacePtr fontFaceRegular;

    struct {
        EntityPtr hover;
    } selected;

    struct {
        bool show{false};
        EntityPtr entity;
        Vector2i pos;
    } contextMenu;

    struct {
        AssetImagePtr info;
        AssetImagePtr approach;
        AssetImagePtr rotation;
        AssetImagePtr target;
    } images;

    struct {
        MessageShipMovement::Request req;
    } movement;
};
} // namespace Engine
