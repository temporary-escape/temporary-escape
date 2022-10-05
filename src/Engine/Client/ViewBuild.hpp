#pragma once

#include "../Assets/Registry.hpp"
#include "../Graphics/Canvas.hpp"
#include "../Graphics/Nuklear.hpp"
#include "../Gui/GuiBlockSelector.hpp"
#include "../Scene/Scene.hpp"
#include "View.hpp"

namespace Engine {
class ViewBuild : public View {
public:
    explicit ViewBuild(const Config& config, VulkanDevice& vulkan, Scene::Pipelines& scenePipelines, Registry& registry,
                       Canvas& canvas, FontFamily& font, Nuklear& nuklear);
    ~ViewBuild() = default;

    void update(float deltaTime) override;
    void render(const Vector2i& viewport, Renderer& renderer) override;
    void renderCanvas(const Vector2i& viewport) override;
    const Skybox* getSkybox() override;
    void eventUserInput(const UserInput::Event& event) override;

    const Config& config;
    VulkanDevice& vulkan;
    Scene::Pipelines& scenePipelines;
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
