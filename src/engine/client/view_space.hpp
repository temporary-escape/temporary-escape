#pragma once

#include "../assets/registry.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../graphics/skybox.hpp"
#include "../gui/gui_block_selector.hpp"
#include "../scene/scene.hpp"
#include "view.hpp"

namespace Engine {
class Client;

class ViewSpace : public View {
public:
    explicit ViewSpace(const Config& config, VulkanDevice& vulkan, Registry& registry, Canvas& canvas, FontFamily& font,
                       Nuklear& nuklear, Skybox& skybox, Client& client);
    ~ViewSpace() = default;

    void update(float deltaTime) override;
    void render(const Vector2i& viewport, Renderer& renderer) override;
    void renderCanvas(const Vector2i& viewport) override;
    void eventUserInput(const UserInput::Event& event) override;

private:
    const Config& config;
    VulkanDevice& vulkan;
    Registry& registry;
    Canvas& canvas;
    FontFamily& font;
    Nuklear& nuklear;
    Skybox& skyboxSystem;
    Client& client;
};
} // namespace Engine
