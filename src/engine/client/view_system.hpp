#pragma once

#include "../assets/registry.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../graphics/skybox.hpp"
#include "../scene/scene.hpp"
#include "../server/world.hpp"
#include "../utils/stop_token.hpp"
#include "view.hpp"

namespace Engine {
class ENGINE_API Client;
class ENGINE_API Game;

class ENGINE_API ViewSystem : public View {
public:
    explicit ViewSystem(Game& parent, const Config& config, Renderer& renderer, Registry& registry, Client& client,
                        Gui& gui, FontFamily& font);
    ~ViewSystem() = default;

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
    Scene& getScene() override;

    void load();
    void load(const std::string& galaxyId, const std::string& systemId);

private:
    void clear();
    void fetchCurrentLocation(const StopToken& stop);
    void fetchSectors(const StopToken& stop, const std::string& token);
    void fetchPlanetaryBodiesPage(const StopToken& stop, const std::string& token);
    void updateSystem();
    void clearEntities();
    void createEntityPositions();
    void createEntityCursor();
    void createEntitiesBodies();

    Game& parent;
    const Config& config;
    Registry& registry;
    Client& client;
    Gui& gui;
    FontFamily& font;
    Skybox skybox;
    Scene scene;
    ComponentCamera* camera{nullptr};

    struct {
        std::string galaxyId;
        std::string systemId;
        std::string sectorId;
    } location;

    struct {
        std::string name;
        std::unordered_map<std::string, PlanetaryBodyData> planets;
        std::unordered_map<std::string, SectorData> sectors;
        std::vector<std::variant<PlanetaryBodyData*, SectorData*>> bodies;
    } system;

    struct {
        std::vector<EntityPtr> bodies;
        std::vector<EntityPtr> orbits;
        EntityPtr icons;
        EntityPtr cursor;
        EntityPtr positions;
        EntityPtr names;
    } entities;

    struct {
        ImagePtr systemPlanet;
        ImagePtr systemMoon;
        ImagePtr iconSelect;
    } images;

    struct {
        const SystemData* hover{nullptr};
        const SystemData* selected{nullptr};
    } input;

    bool loading{false};
    float loadingValue{0.0f};
    StopToken stopToken;
};
} // namespace Engine
