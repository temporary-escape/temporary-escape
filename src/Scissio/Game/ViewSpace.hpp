#pragma once

#include "../Config.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Scene/Scene.hpp"
#include "../Utils/EventBus.hpp"
#include "View.hpp"

namespace Scissio {
class SCISSIO_API ViewSpace : public View {
public:
    explicit ViewSpace(const Config& config, EventBus& eventBus, Renderer& renderer, Scene& scene);
    virtual ~ViewSpace() = default;

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
    Renderer& renderer;
    Scene& scene;

    EntityPtr entitySelected;

    bool cameraMove[6];
    Vector2 cameraRotation;
    bool cameraRotate;
    Vector2 mousePosOld;
};
} // namespace Scissio
