#pragma once

#include "../assets/registry.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../graphics/skybox.hpp"
#include "../gui/gui_context_menu.hpp"
#include "../gui/gui_modal_loading.hpp"
#include "../scene/scene.hpp"
#include "../server/world.hpp"
#include "../utils/stop_token.hpp"
#include "view.hpp"

namespace Engine {
class Client;

class ViewSystem : public View {
public:
    explicit ViewSystem(const Config& config, VulkanDevice& vulkan, Scene::Pipelines& scenePipelines,
                        Registry& registry, Client& client);
    ~ViewSystem() = default;

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

    void load();

private:
    void fetchCurrentLocation(const StopToken& stop);
    void fetchSectors(const StopToken& stop, const std::string& token);
    void fetchPlanetaryBodiesPage(const StopToken& stop, const std::string& token);
    void updateSystem();
    void clearEntities();
    void createEntitiesPlanets();
    const SystemData* rayCast(const Vector2& mousePos);

    const Config& config;
    VulkanDevice& vulkan;
    Registry& registry;
    Client& client;
    Skybox skybox;
    Scene scene;
    std::shared_ptr<ComponentCamera> camera;

    struct {
        std::string galaxyId;
        std::string systemId;
        std::string sectorId;
    } location;

    struct {
        std::string name;
        std::unordered_map<std::string, PlanetaryBodyData> bodies;
        std::unordered_map<std::string, SectorData> sectors;
    } system;

    struct {
        EntityPtr iconPointCloud;
        EntityPtr orbitalRings;
    } entities;

    struct {
        ImagePtr systemPlanet;
        ImagePtr systemMoon;
    } images;

    struct {
        const SystemData* hover{nullptr};
        const SystemData* selected{nullptr};
    } input;

    struct {
        GuiModalLoading modalLoading{"System Map"};
        GuiContextMenu contextMenu;

        Vector2i oldMousePos;
    } gui;

    bool loading{false};
    float loadingValue{0.0f};
    StopToken stopToken;
};
} // namespace Engine