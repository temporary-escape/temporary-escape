#pragma once

#include "../Math/Vector.hpp"
#include "../Platform/Enums.hpp"
#include "Renderer.hpp"

namespace Engine {
class Client;

class ENGINE_API View {
public:
    virtual void render(const Vector2i& viewport) = 0;
    virtual void eventMouseMoved(const Vector2i& pos) = 0;
    virtual void eventMousePressed(const Vector2i& pos, MouseButton button) = 0;
    virtual void eventMouseReleased(const Vector2i& pos, MouseButton button) = 0;
    virtual void eventMouseScroll(int xscroll, int yscroll) = 0;
    virtual void eventKeyPressed(Key key, Modifiers modifiers) = 0;
    virtual void eventKeyReleased(Key key, Modifiers modifiers) = 0;
};

/*class View {
public:
    explicit View(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                  Client& client);

    void render(const Vector2i& viewport);
    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventMouseScroll(int xscroll, int yscroll);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);

private:
    void loadGalaxyMap();
    void loadSystemMap();
    void renderGalaxyMap(const Vector2i& viewport);
    void renderSystemMap(const Vector2i& viewport);

    const Config& config;
    Canvas2D& canvas;
    AssetManager& assetManager;
    Renderer& renderer;
    Client& client;
    GuiContext gui;

    AssetFontFacePtr fontFaceRegular;

    class OrthoCamera {
    public:
        OrthoCamera();
        void moveTo(const Vector2& pos);
        void eventMouseMoved(const Vector2i& pos);
        void eventMousePressed(const Vector2i& pos, MouseButton button);
        void eventMouseReleased(const Vector2i& pos, MouseButton button);
        void eventMouseScroll(int xscroll, int yscroll);
        void stopMove();

        Vector2 worldToScreen(const Vector2& other);
        Vector2 screenToWorld(const Vector2& other);

    private:
        Matrix4 transform;
        Vector2 position;
        bool move;
        float zoom;
        float zoomMin;
        float zoomMax;
        Vector2 mousePosOld;
    };

    struct CameraInternal {
        // bool move[6] = {false};
        Vector3 pos{0.0f};
        Vector2 rotation{0.0f};
        bool rotate{false};
        Vector2 mousePosOld;
        float zoom{3.0f};
    } camera;

    struct GalaxyMapInternal {
        bool active{false};
        std::unordered_map<std::string, SystemData> systems;
        std::unordered_map<std::string, RegionData> regions;
        PlayerLocationData location;
        AssetImagePtr starImage;
        AssetImagePtr markerImage;
        OrthoCamera camera;
    } galaxyMap;

    struct SystemMapInternal {
        bool active{false};
        std::unordered_map<std::string, SectorPlanetData> planets;
        AssetImagePtr planetImage;
        AssetImagePtr starImage;
        AssetImagePtr markerImage;
        OrthoCamera camera;
    } systemMap;
};*/
} // namespace Engine
