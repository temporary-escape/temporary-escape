#pragma once

#include "../Assets/AssetImage.hpp"
#include "../Gui/GuiContext.hpp"
#include "../Server/Schemas.hpp"
#include "View.hpp"
#include "Widgets.hpp"

namespace Engine {
class ENGINE_API ViewMap : public View {
public:
    explicit ViewMap(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                     Client& client, Widgets& widgets, Store& store);

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
    // void fetchPlayerLocation();
    // void fetchGalaxy();
    // void fetchRegions();
    // void fetchSystems();
    // void fetchSystemPlanets();
    void reconstruct();
    // void renderSystems();
    // void renderCurrentPositionGalaxy();
    void renderCurrentPositionInfo();

    const Config& config;
    Canvas2D& canvas;
    AssetManager& assetManager;
    Renderer& renderer;
    Client& client;
    Widgets& widgets;
    Store& store;

    bool loading{false};
    std::unique_ptr<Scene> scene;
    EntityPtr entityCamera;

    AssetFontFacePtr fontFaceRegular;

    std::vector<EntityPtr> entitiesSystems;
    std::vector<EntityPtr> entitiesRegions;

    /*struct Data {
        // Player
        PlayerLocationData location;

        // Galaxy
        GalaxyData galaxy;
        std::unordered_map<std::string, SystemData> systems;
        std::unordered_map<std::string, RegionData> regions;

        // System
        std::unordered_map<std::string, SectorPlanetData> planets;
    } data;*/

    struct Images {
        AssetImagePtr currentPosition;
    } images;

    struct TexturesInternal {
        AssetTexturePtr star;
    } textures;
};
} // namespace Engine
