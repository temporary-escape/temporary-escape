#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Config.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Gui/GuiContext.hpp"
#include "../Network/NetworkClient.hpp"
#include "../Scene/Camera.hpp"
#include "../Scene/Scene.hpp"
#include "Store.hpp"
#include "View.hpp"

namespace Scissio {
class SCISSIO_API ViewMap : public View, public Store::Listener {
public:
    explicit ViewMap(const Config& config, Network::Client& client, Store& store, AssetManager& assetManager);
    virtual ~ViewMap() = default;

    void update(const Vector2i& viewport) override;
    void render(const Vector2i& viewport, Renderer& renderer) override;
    void renderCanvas(const Vector2i& viewport, Canvas2D& canvas, GuiContext& gui) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventMouseScroll(int xscroll, int yscroll) override;

private:
    const Config& config;
    Network::Client& client;
    AssetManager& assetManager;
    Scene scene;
    Camera camera;

    Vector2i viewport;
    EntityPtr galaxy;
    EntityPtr dust;

    bool cameraMove;
    Vector2i mousePosDelta;
    Vector2i mousePos;

    bool loading;
};
} // namespace Scissio
