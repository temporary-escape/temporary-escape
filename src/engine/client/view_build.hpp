#pragma once

#include "../assets/registry.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../scene/scene.hpp"
#include "view.hpp"

namespace Engine {
class ViewBuild : public View {
public:
    explicit ViewBuild(const Config& config, Renderer& renderer, Registry& registry, Gui& gui);
    ~ViewBuild() = default;

    void update(float deltaTime) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;
    void onEnter() override;
    void onExit() override;
    const Renderer::Options& getRenderOptions() override;
    Scene& getRenderScene() override;
    const Skybox& getRenderSkybox() override;

private:
    void createScene();

    const Config& config;
    Registry& registry;
    Gui& gui;

    Vector2 raycastScreenPos;
    std::optional<Grid::RayCastResult> raycastResult;

    Skybox skybox;
    Scene scene;
    EntityPtr entityShip;
    EntityPtr entityHelperAdd;
};
} // namespace Engine
