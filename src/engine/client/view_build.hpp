#pragma once

#include "../assets/registry.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../scene/scene.hpp"
#include "view.hpp"

namespace Engine {
class ViewBuild : public View {
public:
    explicit ViewBuild(const Config& config, Renderer& renderer, Registry& registry);
    ~ViewBuild() = default;

    void update(float deltaTime) override;
    void render(const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;
    void onEnter() override;
    void onExit() override;

    const Config& config;
    Renderer& renderer;
    Registry& registry;

    Vector2 raycastScreenPos;
    std::optional<Grid::RayCastResult> raycastResult;

    Skybox skybox;
    Scene scene;
    EntityPtr entityShip;
    EntityPtr entityHelperAdd;
};
} // namespace Engine
