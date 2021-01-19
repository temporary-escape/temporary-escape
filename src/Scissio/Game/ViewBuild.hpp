#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Config.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Gui/GuiContext.hpp"
#include "../Gui/GuiWindow.hpp"
#include "../Scene/ComponentCamera.hpp"
#include "../Scene/Scene.hpp"
#include "../Utils/EventBus.hpp"
#include "View.hpp"
#include "WindowBlockSelector.hpp"
#include "WindowBuildMenu.hpp"

namespace Scissio {
class SCISSIO_API ViewBuild : public View {
public:
    explicit ViewBuild(const Config& config, EventBus& eventBus, AssetManager& assetManager, Renderer& renderer,
                       Canvas2D& canvas);
    virtual ~ViewBuild() = default;

    void render(const Vector2i& viewport) override;
    void canvas(const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventKeyPressed(Key key) override;
    void eventKeyReleased(Key key) override;

private:
    const Config& config;
    EventBus& eventBus;
    AssetManager& assetManager;
    Renderer& renderer;
    GuiContext gui;
    Scene scene;

    Vector2i viewport;
    std::shared_ptr<ComponentCamera> camera;
    EntityPtr ship;
    EntityPtr preview;

    bool cameraMove[6];
    Vector2 cameraRotation;
    bool cameraRotate;
    Vector2 mousePosOld;
    Vector2 mousePos;

    WindowBlockSelector blockSelector;
    WindowBuildMenu menu;
};
} // namespace Scissio
