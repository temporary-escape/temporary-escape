#pragma once

#include "../assets/registry.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../gui/gui_block_selector.hpp"
#include "../scene/scene.hpp"
#include "view.hpp"

namespace Engine {
class ViewBuild : public View {
public:
    explicit ViewBuild(const Config& config, VulkanDevice& vulkan, Scene::Pipelines& scenePipelines, Registry& registry,
                       Canvas& canvas, FontFamily& font, Nuklear& nuklear);
    ~ViewBuild() = default;

    void update(float deltaTime) override;
    void render(const Vector2i& viewport, Renderer& renderer) override;
    void renderCanvas(const Vector2i& viewport) override;
    void eventUserInput(const UserInput::Event& event) override;

    const Config& config;
    VulkanDevice& vulkan;
    Registry& registry;
    Canvas& canvas;
    FontFamily& font;
    Nuklear& nuklear;

    Vector2 raycastScreenPos;
    std::optional<Grid::RayCastResult> raycastResult;

    Skybox skybox;
    Scene scene;
    EntityPtr entityShip;
    EntityPtr entityHelperAdd;

    GuiBlockSelector guiBlockSelector;
};
} // namespace Engine
