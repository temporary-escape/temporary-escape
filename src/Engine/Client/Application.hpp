#pragma once

#include "../Audio/AudioContext.hpp"
#include "../Database/Database.hpp"
#include "../Graphics/RendererCanvas.hpp"
#include "../Graphics/RendererScenePbr.hpp"
#include "../Graphics/RendererThumbnail.hpp"
#include "../Gui/GuiManager.hpp"
#include "../Gui/Windows/GuiWindowMainMenu.hpp"
#include "../Server/Server.hpp"
#include "../Utils/PerformanceRecord.hpp"
#include "../Vulkan/VulkanRenderer.hpp"
#include "Editor.hpp"
#include "Game.hpp"
#include <queue>

namespace Engine {
struct ENGINE_API Status {
    std::string message;
    float value{0.0f};
};

class ENGINE_API Application : public VulkanRenderer, public Nuklear::EventCallback {
public:
    explicit Application(Config& config);
    virtual ~Application();

    void render(const Vector2i& viewport, float deltaTime) override;

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventWindowResized(const Vector2i& size) override;
    void eventCharTyped(uint32_t code) override;
    void eventWindowBlur() override;
    void eventWindowFocus() override;

private:
    void loadSounds();
    void renderStatus(const Vector2i& viewport);
    void renderVersion(const Vector2i& viewport);
    void renderFrameTime(const Vector2i& viewport);
    void checkForClientScene();
    void loadAssets();
    void createEditor();
    void createThumbnails();
    void createRegistry();
    void createVoxelShapeCache();
    void compressAssets();
    void createSceneRenderer(const Vector2i& viewport);
    void createRenderers();
    void loadNextAssetInQueue(AssetsManager::LoadQueue::const_iterator next);
    void startDatabase();
    void startServer();
    void startClient();
    void startSinglePlayer();
    void startEditor();
    void createPlanetLowResTextures(RendererPlanetSurface& rendererPlanetSurface);
    void createPlanetLowResTextures();
    void createBlockThumbnails(RendererThumbnail& thumbnailRenderer);
    void createEmptyThumbnail(RendererThumbnail& thumbnailRenderer);
    void createPlanetThumbnails(RendererThumbnail& thumbnailRenderer);
    bool shouldBlit() const;

    void nuklearOnClick(bool push) override;

    Config& config;

    AudioContext audio;
    AudioSource audioSource;
    Canvas canvas;
    FontFamily font;
    Nuklear nuklear;
    Status status;
    VulkanQueryPool renderQueryPool;
    RendererCanvas rendererCanvas;
    Canvas2 canvas2;
    GuiManager guiManager;

    /*struct {
        GuiMainMenu mainMenu;
        GuiCreateProfile createProfile;
        GuiMainSettings mainSettings;
        GuiKeepSettings keepSettings;
    } gui;*/

    std::unique_ptr<AssetsManager> assetsManager;
    std::unique_ptr<Database> db;
    std::unique_ptr<Server> server;
    std::thread serverThread;
    std::unique_ptr<RendererSkybox> rendererSkybox;
    std::unique_ptr<RendererPlanetSurface> rendererPlanetSurface;
    std::unique_ptr<RenderResources> renderResources;
    std::unique_ptr<RendererScenePbr> renderer;
    std::unique_ptr<VoxelShapeCache> voxelShapeCache;
    std::unique_ptr<Client> client;
    PlayerLocalProfile playerLocalProfile;
    std::unique_ptr<Game> game;
    std::unique_ptr<Editor> editor;

    std::future<std::function<void()>> future;
    std::promise<std::function<void()>> promise;
    std::atomic<bool> shouldStop{false};
    Vector2i mousePos;
    bool editorOnly{false};
    int shouldBlitCount{0};

    struct {
        PerformanceRecord frameTime;
        PerformanceRecord renderTime;
    } perf;

    struct {
        AudioBuffer uiClick;
    } sounds;
};
} // namespace Engine