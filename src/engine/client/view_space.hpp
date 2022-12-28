#pragma once

#include "../assets/registry.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../graphics/skybox.hpp"
#include "../scene/scene.hpp"
#include "view.hpp"

namespace Engine {
class Client;

class ViewSpace : public View {
public:
    explicit ViewSpace(const Config& config, VulkanDevice& vulkan, Registry& registry, Skybox& skybox, Client& client);
    ~ViewSpace() = default;

    void update(float deltaTime) override;
    void render(const Vector2i& viewport, Renderer& renderer) override;
    void renderCanvas(const Vector2i& viewport, Canvas& canvas) override;
    void renderGui(const Vector2i& viewport, Nuklear& nuklear) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;
    void onEnter() override;
    void onExit() override;

private:
    const Config& config;
    VulkanDevice& vulkan;
    Registry& registry;
    Skybox& skyboxSystem;
    Client& client;
};
} // namespace Engine