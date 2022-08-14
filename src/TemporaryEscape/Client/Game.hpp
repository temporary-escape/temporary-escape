#pragma once

#include "../Assets/Registry.hpp"
#include "../Assets/VoxelShapeCache.hpp"
#include "../Config.hpp"
#include "../Font/FontFamily.hpp"
#include "../Future.hpp"
#include "../Utils/AsyncTask.hpp"
#include "../Vulkan/VulkanDevice.hpp"
#include "Canvas.hpp"
#include "Nuklear.hpp"
#include "ViewBuild.hpp"

namespace Engine {
struct Status {
    std::string message{"Default message"};
    float value{1.0f};
};

class ENGINE_API Game : public View {
public:
    explicit Game(const Config& config, VulkanDevice& vulkan, Registry& registry, Canvas& canvas, FontFamily& font,
                  Scene::Pipelines& scenePipelines);
    virtual ~Game();

    void update(float deltaTime) override;
    void renderPbr(const Vector2i& viewport) override;
    void renderFwd(const Vector2i& viewport) override;
    void renderCanvas(const Vector2i& viewport) override;
    const Skybox* getSkybox() override;

    void eventUserInput(const UserInput::Event& event) override;
    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventMouseScroll(int xscroll, int yscroll);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);
    void eventWindowResized(const Vector2i& size);

private:
    // void eventInitDone();

    const Config& config;
    VulkanDevice& vulkan;
    Registry& registry;
    Canvas& canvas;
    FontFamily& font;
    Scene::Pipelines& scenePipelines;
    // bool shouldLoadShaders{false};
    UserInput userInput;
    Nuklear nuklear;
    // std::unique_ptr<Registry> registry;
    // std::unique_ptr<AsyncTask> registryInit;
    // bool registryInitFinished{false};
    // bool registryLoadFinished{false};
    std::unique_ptr<ViewBuild> viewBuild;
    View* view{nullptr};
    // std::string statusText;
    // float statusValue{0.0f};
    // bool viewReady{false};
};
} // namespace Engine
