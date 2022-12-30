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
#include "client.hpp"
#include "stats.hpp"
#include "view_build.hpp"
#include "view_galaxy.hpp"
#include "view_space.hpp"
#include "view_system.hpp"

namespace Engine {
class ENGINE_API TransactionalDatabase;

struct ENGINE_API Status {
    std::string message{"Default message"};
    float value{1.0f};
};

class ENGINE_API Game : public UserInput {
public:
    explicit Game(const Config& config, VulkanRenderer& vulkan, Registry& registry, Canvas& canvas, FontFamily& font,
                  SkyboxGenerator& skyboxGenerator, Status& status);
    virtual ~Game();

    void update(float deltaTime);
    void render(const Vector2i& viewport, Renderer& renderer);
    void renderCanvas(const Vector2i& viewport);

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

    bool hasView() const {
        return view != nullptr;
    }

private:
    // void eventInitDone();

    const Config& config;
    VulkanRenderer& vulkan;
    Registry& registry;
    Canvas& canvas;
    FontFamily& font;
    // Scene::Pipelines& scenePipelines;
    SkyboxGenerator& skyboxGenerator;
    Status& status;
    Stats stats;
    // bool shouldLoadShaders{false};
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
    std::unique_ptr<ViewSystem> viewSystem;
    View* view{nullptr};
    // std::string statusText;
    // float statusValue{0.0f};
    // bool viewReady{false};
};
} // namespace Engine
