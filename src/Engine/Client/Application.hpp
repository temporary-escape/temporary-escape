#pragma once

#include "../Audio/AudioContext.hpp"
#include "../Font/FontFamilyDefault.hpp"
#include "../Graphics/RendererCanvas.hpp"
#include "../Gui/GuiManager.hpp"
#include "../Gui/Windows/GuiWindowCreateProfile.hpp"
#include "../Gui/Windows/GuiWindowCreateSave.hpp"
#include "../Gui/Windows/GuiWindowGameMenu.hpp"
#include "../Gui/Windows/GuiWindowLoadSave.hpp"
#include "../Gui/Windows/GuiWindowLogIn.hpp"
#include "../Gui/Windows/GuiWindowMainMenu.hpp"
#include "../Gui/Windows/GuiWindowMultiplayerSettings.hpp"
#include "../Gui/Windows/GuiWindowServerBrowser.hpp"
#include "../Gui/Windows/GuiWindowSettings.hpp"
#include "../Server/Server.hpp"
#include "../Utils/PerformanceRecord.hpp"
#include "../Vulkan/VulkanRenderer.hpp"
#include "BannerImage.hpp"
#include "Client.hpp"
#include "View.hpp"
#include <queue>

namespace Engine {
struct ENGINE_API Status {
    std::string message;
    float value{0.0f};
};

class ENGINE_API MatchmakerClient;
class ENGINE_API ViewSpace;
class ENGINE_API ViewGalaxy;
class ENGINE_API ViewSystem;
class ENGINE_API ViewBuild;
class ENGINE_API Database;
class ENGINE_API RendererBackground;
class ENGINE_API RendererScenePbr;
class ENGINE_API RendererThumbnail;
class ENGINE_API NetworkUdpClient;

class ENGINE_API Application : public VulkanRenderer {
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
    void eventWindowInputBegin() override;
    void eventWindowInputEnd() override;

private:
    void checkOnlineServices();
    void quitToMenu();
    void shutdownServerSide();
    void shutdownClientSide();
    void shutdownViews();
    void stopServerSide();
    void stopClientSide();
    void shutdownDone();
    void loadProfile();
    void renderStatus(const Vector2i& viewport);
    void renderVersion(const Vector2i& viewport);
    void renderFrameTime(const Vector2i& viewport);
    void renderBanner(const Vector2i& viewport);
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
    void startServer();
    void startClientToServer();
    void startClient();
    void openMultiplayerSettings();
    void startMultiPlayerHosted();
    void startSinglePlayer();
    void startConnectServer(const std::string& serverId);
    void startEditor();
    void createPlanetLowResTextures(RendererPlanetSurface& rendererPlanetSurface);
    void createPlanetLowResTextures();
    void createBlockThumbnails(RendererThumbnail& thumbnailRenderer);
    void createEmptyThumbnail(RendererThumbnail& thumbnailRenderer);
    void createPlanetThumbnails(RendererThumbnail& thumbnailRenderer);
    bool isViewsInputSuspended() const;

    Config& config;

    AudioContext audio;
    AudioSource audioSource;
    FontFamilyDefault font;
    Status status;
    VulkanQueryPool renderQueryPool;
    RendererCanvas rendererCanvas;
    Canvas canvas;
    GuiManager guiManager;
    BannerImage bannerTexture;
    MatchmakerClient matchmakerClient;

    struct {
        GuiWindowMainMenu* mainMenu{nullptr};
        GuiWindowCreateProfile* createProfile{nullptr};
        GuiWindowSettings* settings{nullptr};
        GuiWindowServerBrowser* serverBrowser{nullptr};
        GuiWindowLoadSave* loadSave{nullptr};
        GuiWindowCreateSave* createSave{nullptr};
        GuiWindowGameMenu* gameMenu{nullptr};
        GuiWindowLogIn* logIn{nullptr};
        GuiWindowMultiplayerSettings* multiplayerSettings{nullptr};
    } gui;

    struct {
        ViewSpace* space{nullptr};
        ViewGalaxy* galaxy{nullptr};
        ViewSystem* system{nullptr};
        ViewBuild* editor{nullptr};
    } view;

    std::unique_ptr<AssetsManager> assetsManager;
    std::unique_ptr<Server> server;
    std::unique_ptr<RendererBackground> rendererBackground;
    std::unique_ptr<RenderResources> renderResources;
    std::unique_ptr<RendererScenePbr> renderer;
    std::unique_ptr<VoxelShapeCache> voxelShapeCache;
    std::unique_ptr<Client> client;
    std::unique_ptr<ViewContext> views;
    PlayerLocalProfile playerLocalProfile;

    std::future<std::function<void()>> future;
    std::promise<std::function<void()>> promise;
    std::atomic<bool> shouldStop{false};
    Vector2i mousePos{0, 0};
    bool editorOnly{false};

    struct {
        PerformanceRecord frameTime;
        PerformanceRecord renderTime;
    } perf;

    Server::Options serverOptions;
    std::string connectAddress{"::1"};
    std::string connectServerId;
};
} // namespace Engine
