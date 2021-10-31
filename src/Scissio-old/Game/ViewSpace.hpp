#pragma once

#include "../Config.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Gui/GuiContext.hpp"
#include "../Network/NetworkClient.hpp"
#include "../Scene/Camera.hpp"
#include "../Scene/Scene.hpp"
#include "View.hpp"

namespace Scissio {
class SCISSIO_API ViewSpace : public View {
public:
    explicit ViewSpace(const Config& config, Network::Client& client, Scene& scene);
    virtual ~ViewSpace() = default;

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
    Scene& scene;
    Camera camera;

    EntityPtr entitySelected;

    bool cameraMove[6];
    Vector2 cameraRotation;
    bool cameraRotate;
    Vector2 mousePosOld;
};
} // namespace Scissio
