#pragma once

#include "../assets/registry.hpp"
#include "../future.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../graphics/skybox.hpp"
#include "../math/voronoi_diagram.hpp"
#include "../scene/scene.hpp"
#include "../server/world.hpp"
#include "../utils/stop_token.hpp"
#include "view.hpp"

namespace Engine {
class ENGINE_API Client;
class ENGINE_API Game;

class ENGINE_API ViewGalaxy : public View {
public:
    explicit ViewGalaxy(Game& parent, const Config& config, Renderer& renderer, Registry& registry, Client& client,
                        Gui& gui, FontFamily& font);
    ~ViewGalaxy() = default;

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

    void load();

private:
    void fetchCurrentLocation(const StopToken& stop);
    void fetchGalaxyInfo(const StopToken& stop);
    void fetchFactionsPage(const StopToken& stop, const std::string& token);
    void fetchSystemsPage(const StopToken& stop, const std::string& token);
    void fetchRegionsPage(const StopToken& stop, const std::string& token);
    void updateGalaxy();
    void clearEntities();
    void createEntitiesRegions();
    void calculateBackground();
    void createBackground(const VoronoiResult& voronoi);

    Game& parent;
    const Config& config;
    Registry& registry;
    Client& client;
    Gui& gui;
    FontFamily& font;
    Skybox skybox;
    Scene scene;

    struct {
        std::string galaxyId;
        std::string systemId;
        std::string sectorId;
    } location;

    struct {
        std::string name;
        std::unordered_map<std::string, SystemData> systems;
        std::unordered_map<std::string, RegionData> regions;
        std::vector<const SystemData*> systemsOrdered;
    } galaxy;

    std::unordered_map<std::string, FactionData> factions;

    struct {
        std::shared_ptr<Entity> camera;
        std::unordered_map<std::string, EntityPtr> regions;
        std::unordered_map<std::string, EntityPtr> labels;
        std::unordered_map<std::string, EntityPtr> factions;
        EntityPtr positions;
        EntityPtr cursor;
        EntityPtr voronoi;
        EntityPtr names;
    } entities;

    struct {
        TexturePtr systemStar;
    } textures;

    struct {
        ImagePtr iconSelect;
    } images;

    bool loading{false};
    float loadingValue{0.0f};
    StopToken stopToken;
    Future<VoronoiResult> futureVoronoi;
};
} // namespace Engine
