#pragma once

#include "../assets/registry.hpp"
#include "../assets/voxel_shape_cache.hpp"
#include "../config.hpp"
#include "../database/rocks_db.hpp"
#include "../font/font_family.hpp"
#include "../future.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../graphics/skybox_generator.hpp"
#include "../server/server.hpp"
#include "../vulkan/vulkan_device.hpp"
#include "client.hpp"
#include "stats.hpp"
#include "view_build.hpp"
#include "view_galaxy.hpp"
#include "view_space.hpp"

namespace Engine {
class ENGINE_API TransactionalDatabase;

struct ENGINE_API Status {
    std::string message{"Default message"};
    float value{1.0f};
};

class ENGINE_API Game : public View {
public:
    explicit Game(const Config& config, VulkanDevice& vulkan, Registry& registry, Canvas& canvas, FontFamily& font,
                  Scene::Pipelines& scenePipelines, SkyboxGenerator& skyboxGenerator, Status& status);
    virtual ~Game();

    void update(float deltaTime) override;
    void render(const Vector2i& viewport, Renderer& renderer) override;
    void renderCanvas(const Vector2i& viewport) override;

    void eventUserInput(const UserInput::Event& event) override;
    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventMouseScroll(int xscroll, int yscroll);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);
    void eventWindowResized(const Vector2i& size);
    void eventCharTyped(uint32_t code);

    bool hasView() const {
        return view != nullptr;
    }

private:
    // void eventInitDone();

    const Config& config;
    VulkanDevice& vulkan;
    Registry& registry;
    Canvas& canvas;
    FontFamily& font;
    Scene::Pipelines& scenePipelines;
    SkyboxGenerator& skyboxGenerator;
    Status& status;
    Stats stats;
    // bool shouldLoadShaders{false};
    UserInput userInput;
    Nuklear nuklear;
    Server::Certs serverCerts;

    std::unique_ptr<TransactionalDatabase> db;
    std::unique_ptr<Server> server;
    Future<void> serverLoad;
    std::unique_ptr<Client> client;
    Future<void> clientLoad;
    // std::unique_ptr<Registry> registry;
    // std::unique_ptr<AsyncTask> registryInit;
    // bool registryInitFinished{false};
    // bool registryLoadFinished{false};
    std::unique_ptr<ViewBuild> viewBuild;
    std::unique_ptr<ViewSpace> viewSpace;
    std::unique_ptr<ViewGalaxy> viewGalaxy;
    View* view{nullptr};
    // std::string statusText;
    // float statusValue{0.0f};
    // bool viewReady{false};
};
} // namespace Engine
