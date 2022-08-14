#pragma once

#include "../Assets/AssetImage.hpp"
#include "../Future.hpp"
#include "../Gui/GuiContext.hpp"
#include "../Server/Schemas.hpp"
#include "View.hpp"
#include "Widgets.hpp"

namespace Engine {
class ENGINE_API ViewMap : public View {
public:
    explicit ViewMap(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                     Client& client, Widgets& widgets);

    void load();
    void render(const Vector2i& viewport) override;
    void renderGui(const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;

private:
    struct ReconstructData {
        std::vector<EntityPtr> entitiesSystems;
        std::vector<EntityPtr> entitiesRegions;
        std::vector<EntityPtr> entitiesFactions;
    };

    void fetchGalaxy();
    void fetchRegions();
    void fetchFactions();
    void fetchSystems();
    void fetchSystemPlanets();

    void reconstructWith(const ReconstructData& res);
    ReconstructData reconstruct();
    Future<ReconstructData> reconstructAsync();
    // void renderSystems();
    // void renderCurrentPositionGalaxy();
    void renderRegionLabels();
    void renderRegionSystemLabels();
    void renderCurrentPositionInfo();
    void renderCurrentPositionMarker();

    // EntityPtr createFactionBorders();

    const Config& config;
    Canvas2D& canvas;
    AssetManager& assetManager;
    Renderer& renderer;
    Client& client;
    Widgets& widgets;

    bool loading{false};
    std::unique_ptr<Scene> scene;
    // EntityPtr entityCamera;

    AssetFontFacePtr fontFaceRegular;

    // std::vector<EntityPtr> entitiesSystems;
    // std::vector<EntityPtr> entitiesRegions;
    // EntityPtr entityBorders;

    std::chrono::time_point<std::chrono::steady_clock> lastTimePoint;

    Future<ReconstructData> reconstructFuture;

    struct {
        // Galaxy
        GalaxyData galaxy;
        std::unordered_map<std::string, SystemData> systems;
        std::unordered_map<std::string, RegionData> regions;
        std::unordered_map<std::string, FactionData> factions;

        // System
        std::unordered_map<std::string, SectorPlanetData> planets;
    } data;

    struct {
        AssetImagePtr currentPosition;
    } images;

    struct {
        AssetTexturePtr star;
    } textures;
};
} // namespace Engine
