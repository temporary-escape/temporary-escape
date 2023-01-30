#pragma once

#include "../assets/registry.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../graphics/skybox.hpp"
#include "../scene/scene.hpp"
#include "view.hpp"

namespace Engine {
class ENGINE_API Client;
class ENGINE_API Game;

class ENGINE_API ViewSpace : public View {
public:
    explicit ViewSpace(Game& parent, const Config& config, Renderer& renderer, Registry& registry, Skybox& skybox,
                       Client& client, Gui& gui);
    ~ViewSpace() = default;

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
    Game& parent;
    const Config& config;
    Registry& registry;
    Skybox& skyboxSystem;
    Client& client;
    Gui& gui;
};
} // namespace Engine
