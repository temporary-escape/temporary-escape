#pragma once

#include "../Assets/Registry.hpp"
#include "../Scene/Scene.hpp"
#include "Canvas.hpp"
#include "Nuklear.hpp"
#include "View.hpp"

namespace Engine {
class ViewBuild : public View {
public:
    explicit ViewBuild(const Config& config, VulkanDevice& vulkan, Scene::Pipelines& scenePipelines, Registry& registry,
                       Canvas& canvas, Nuklear& nuklear);
    ~ViewBuild() = default;

    void update(float deltaTime) override;
    void renderPbr(const Vector2i& viewport) override;
    void renderFwd(const Vector2i& viewport) override;
    void renderCanvas(const Vector2i& viewport) override;
    const Skybox* getSkybox() override;
    void eventUserInput(const UserInput::Event& event) override;

    const Config& config;
    VulkanDevice& vulkan;
    Scene::Pipelines& scenePipelines;
    Registry& registry;
    Canvas& canvas;
    Nuklear& nuklear;

    Vector2 raycastScreenPos;
    std::optional<Grid::RayCastResult> raycastResult;

    Skybox skybox;
    Scene scene;
    EntityPtr entityShip;
    EntityPtr entityHelperAdd;
};
} // namespace Engine
