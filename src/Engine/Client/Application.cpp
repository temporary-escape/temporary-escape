#include "Application.hpp"
#include "../Graphics/RendererBackground.hpp"
#include "../Graphics/RendererScenePbr.hpp"
#include "../Graphics/RendererThumbnail.hpp"
#include "ViewBuild.hpp"
#include "ViewGalaxy.hpp"
#include "ViewSpace.hpp"
#include "ViewSystem.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);
static const auto profileFilename = "profile.xml";

static void saveSettings(Config& config) {
    Xml::toFile(config.userdataPath / "settings.xml", config);
}

#define NEXT(expr)                                                                                                     \
    promise = decltype(promise){};                                                                                     \
    promise.set_value([=]() { (expr); });                                                                              \
    future = promise.get_future()

Application::Application(Config& config) :
    VulkanRenderer{config},
    config{config},
    audio{},
    audioSource{audio.createSource()},
    font{config, *this, 42},
    rendererCanvas{*this},
    canvas{*this},
    guiManager{config, *this, font, config.guiFontSize},
    bannerTexture{*this},
    matchmakerClient{config} {

    // Fix monitor name
    const auto monitors = listSystemMonitors();
    const auto found = std::find_if(monitors.begin(), monitors.end(), [&](const MonitorInfo& monitor) {
        return monitor.name == config.graphics.monitorName;
    });
    const auto primary =
        std::find_if(monitors.begin(), monitors.end(), [&](const MonitorInfo& monitor) { return monitor.primary; });

    if (found == monitors.end()) {
        config.graphics.monitorName = primary->name;
        saveSettings(config);
    }

    const std::string gpuName{getPhysicalDeviceProperties().deviceName};
    if (gpuName.find("UHD Graphics") != std::string::npos) {
        config.graphics.planetTextureSize = 512;
        config.graphics.skyboxSize = 1024;
        config.graphics.ssao = false;
    }

    gui.createProfile = guiManager.addWindow<GuiWindowCreateProfile>();
    gui.createProfile->setOnCreateCallback([this](const GuiWindowCreateProfile::Result& result) {
        try {
            playerLocalProfile.name = result.name;
            playerLocalProfile.secret = randomId();
            Xml::toFile(this->config.userdataPath / profileFilename, playerLocalProfile);
            gui.createProfile->setEnabled(false);

            guiManager.modalSuccess("Success", "User profile created!", [this](const std::string& choice) {
                (void)choice;
                gui.mainMenu->setEnabled(true);
                return true;
            });

        } catch (std::exception& e) {
            guiManager.modalDanger("Error", "Failed to save profile!", [this](const std::string& choice) {
                (void)choice;
                closeWindow();
                return true;
            });

            BACKTRACE(e, "Failed to create user profile");
        }
    });

    gui.mainMenu = guiManager.addWindow<GuiWindowMainMenu>();
    gui.mainMenu->setOnClickNewGame([this]() {
        gui.mainMenu->setEnabled(false);
        gui.createSave->setEnabled(true);
    });
    gui.mainMenu->setOnClickLoadSave([this]() {
        gui.mainMenu->setEnabled(false);
        gui.loadSave->setEnabled(true);
        gui.loadSave->loadInfos();
        gui.loadSave->setMode(MultiplayerMode::Singleplayer);
    });
    gui.mainMenu->setOnClickOnline([this]() {
        gui.mainMenu->setEnabled(false);
        checkOnlineServices();
    });
    gui.mainMenu->setOnClickEditor([this]() {
        gui.mainMenu->setEnabled(false);
        startEditor();
    });
    gui.mainMenu->setOnClickQuit([this]() { closeWindow(); });
    gui.mainMenu->setOnClickSettings([this]() {
        gui.settings->setEnabled(true);
        gui.settings->reset();
        gui.mainMenu->setEnabled(false);
    });

    gui.settings = guiManager.addWindow<GuiWindowSettings>(*this, config, guiManager);
    gui.settings->setOnApply([this]() {
        this->setWindowMode(
            this->config.graphics.windowMode, this->config.graphics.windowSize, this->config.graphics.monitorName);
    });
    gui.settings->setOnSubmit([this](bool value) {
        gui.settings->setEnabled(false);

        if (views) {
            gui.gameMenu->setEnabled(true);
            guiManager.setFocused(*gui.gameMenu);
        } else {
            gui.mainMenu->setEnabled(true);
        }

        if (value) {
            saveSettings(this->config);
        }
    });

    gui.serverBrowser = guiManager.addWindow<GuiWindowServerBrowser>(matchmakerClient, guiManager);
    gui.serverBrowser->setOnClose([this]() {
        gui.mainMenu->setEnabled(true);
        gui.serverBrowser->setEnabled(false);
    });
    gui.serverBrowser->setOnConnect([this](const std::string& serverId) {
        logger.info("Starting connection to the server id: {}", serverId);
        startConnectServer(serverId);
    });
    gui.serverBrowser->setOnCreate([this]() {
        gui.serverBrowser->setEnabled(false);
        gui.loadSave->setEnabled(true);
        gui.loadSave->loadInfos();
        gui.loadSave->setMode(MultiplayerMode::Online);
    });

    gui.createSave = guiManager.addWindow<GuiWindowCreateSave>(guiManager, config.userdataSavesPath);
    gui.createSave->setOnClose([this]() {
        gui.mainMenu->setEnabled(true);
        gui.createSave->setEnabled(false);
    });
    gui.createSave->setOnCreate([this](const GuiWindowCreateSave::Form& form) {
        gui.createSave->setEnabled(false);
        serverOptions.seed = form.seed;
        serverOptions.savePath = form.path;
        if (gui.loadSave->getMode() == MultiplayerMode::Singleplayer) {
            serverOptions.name.clear();
            serverOptions.password.clear();
            startSinglePlayer();
        } else {
            openMultiplayerSettings();
        }
    });

    gui.loadSave = guiManager.addWindow<GuiWindowLoadSave>(guiManager, config.userdataSavesPath);
    gui.loadSave->setOnClose([this]() {
        gui.mainMenu->setEnabled(true);
        gui.loadSave->setEnabled(false);
    });
    gui.loadSave->setOnLoad([this](const Path& path) {
        gui.loadSave->setEnabled(false);
        serverOptions.seed = 0;
        serverOptions.savePath = path;
        if (gui.loadSave->getMode() == MultiplayerMode::Singleplayer) {
            serverOptions.name.clear();
            serverOptions.password.clear();
            startSinglePlayer();
        } else {
            openMultiplayerSettings();
        }
    });
    gui.loadSave->setOnCreate([this]() {
        gui.loadSave->setEnabled(false);
        gui.createSave->setEnabled(true);
    });

    gui.gameMenu = guiManager.addWindow<GuiWindowGameMenu>();
    gui.gameMenu->setOnClickContinue([this]() {
        gui.gameMenu->close();
        guiManager.clearFocused();
    });
    gui.gameMenu->setOnClickSettings([this]() {
        gui.gameMenu->close();
        gui.settings->setEnabled(true);
        gui.settings->reset();
        guiManager.setFocused(*gui.settings);
    });
    gui.gameMenu->setOnClickQuitToMenu([this]() {
        gui.gameMenu->close();
        quitToMenu();
    });
    gui.gameMenu->setOnClickQuitGame([this]() {
        gui.gameMenu->close();
        closeWindow();
    });

    gui.multiplayerSettings = guiManager.addWindow<GuiWindowMultiplayerSettings>();
    gui.multiplayerSettings->setOnClose([this]() { gui.mainMenu->setEnabled(true); });
    gui.multiplayerSettings->setOnStart([this](const GuiWindowMultiplayerSettings::Form& form) {
        gui.multiplayerSettings->setEnabled(false);
        serverOptions.name = form.name;
        serverOptions.password = form.password;
        startMultiPlayerHosted();
    });

    loadProfile();

    VkQueryPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    createInfo.queryCount = MAX_FRAMES_IN_FLIGHT;
    renderQueryPool = createQueryPool(createInfo);

    if (config.autoStart) {
        gui.mainMenu->setEnabled(false);
        startSinglePlayer();
    }
}

Application::~Application() {
    shouldStop.store(true);
    if (future.valid()) {
        try {
            future.get();
        } catch (const std::exception& e) {
            BACKTRACE(e, "fatal error");
        }
    }

    stopServerSide();
}

void Application::render(const Vector2i& viewport, const float deltaTime) {
    // gui.keepSettings.updateProgress(deltaTime);
    // gui.mainMenu.setEnabled(!gui.keepSettings.isEnabled() && !gui.mainSettings.isEnabled() && status.value <= 0.0f);

    if (getSwapChain().getExtent().width != viewport.x || getSwapChain().getExtent().height != viewport.y) {
        logger.warn("Swap chain size does not match");
        return;
    }

    // Create the renderer if it does not exist.
    // Or re-create it if the viewport size does not match.
    if (renderer && renderer->getViewport() != viewport) {
        logger.info("Resizing renderer to: {}", viewport);

        waitQueueIdle();
        waitDeviceIdle();
        renderer.reset();

        createSceneRenderer(viewport);
    }

    const auto t0 = std::chrono::steady_clock::now();

    if (future.valid() && future.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {
        auto fn = future.get();
        fn();
    }

    if (client) {
        client->update(deltaTime);
    }

    rendererCanvas.reset();

    auto vkb = createCommandBuffer();

    const auto queryResult = renderQueryPool.getResult<uint64_t>(0, 2, VK_QUERY_RESULT_64_BIT);
    if (!queryResult.empty()) {
        uint64_t timeDiff = queryResult[1] - queryResult[0];
        timeDiff = static_cast<uint64_t>(static_cast<double>(timeDiff) *
                                         static_cast<double>(getPhysicalDeviceProperties().limits.timestampPeriod));
        const auto timeDiffNano = std::chrono::nanoseconds{timeDiff};
        perf.renderTime.update(timeDiffNano);
        // const auto timeDiffMs = std::chrono::duration_cast<std::chrono::milliseconds>(timeDiffNano);
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    if (getGraphicsQueueFamilyProperties().timestampValidBits) {
        vkb.resetQueryPool(renderQueryPool, 0, 2);
        vkb.writeTimestamp(renderQueryPool, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0);
    }

    if (renderer && !isViewsInputSuspended()) {
        renderer->setMousePos(mousePos);
    }

    /*if (game) {
        game->update(deltaTime);
        game->render(vkb, *renderer, viewport);
    } else if (editor) {
        editor->update(deltaTime);
        editor->render(vkb, *renderer, viewport);
    }*/

    /*if (client) {
        const auto scene = client->getScene();
        if (scene) {
            scene->update(deltaTime);
        }
    }*/

    if (views) {
        views->update(deltaTime, viewport);
        views->render(vkb, *renderer, viewport);
    }

    // GUI
    guiManager.draw(viewport);

    // HUD
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getSwapChainFramebuffer();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValues = {clearColor};

    if (views && views->getCurrent() && renderer->isBlitReady()) {
        renderer->transitionForBlit(vkb);
    }

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    if (views && views->getCurrent() && renderer->isBlitReady()) {
        renderer->blit(vkb);
    }

    // canvas.begin(viewport);

    canvas.begin(viewport);

    renderVersion(viewport);
    renderFrameTime(viewport);

    if (views) {
        views->renderCanvas(canvas, viewport);
    }

    /*if (shouldBlit()) {
        if (game && game->isReady()) {
            // game->renderCanvas(canvas, nuklear, viewport);
        } else if (editor) {
            // editor->renderCanvas(canvas, nuklear, viewport);
        }
    }*/

    if (!views || !views->getCurrent()) {
        if (!status.message.empty()) {
            renderStatus(viewport);
        } else {
            renderBanner(viewport);
        }

        /*nuklear.begin(viewport);
        gui.mainMenu.draw(nuklear, viewport);
        gui.createProfile.draw(nuklear, viewport);
        gui.mainSettings.draw(nuklear, viewport);
        gui.keepSettings.draw(nuklear, viewport);
        nuklear.end();*/

        /*canvas.begin();
        canvas.drawRect({200.0f, 200.0f}, {300.0f, 100.0f}, {1.0f, 0.0f, 0.0f, 1.0f});
        canvas.drawRect({500.0f, 500.0f}, {50.0f, 100.0f}, {0.0f, 1.0f, 0.0f, 1.0f});
        canvas.flush();

        rendererCanvas.render(vkb, canvas, viewport);*/
    }

    canvas.flush();
    rendererCanvas.render(vkb, canvas, viewport);

    guiManager.render(vkb, rendererCanvas, viewport);

    vkb.endRenderPass();

    if (getGraphicsQueueFamilyProperties().timestampValidBits) {
        vkb.writeTimestamp(renderQueryPool, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 1);
    }

    vkb.end();

    submitPresentCommandBuffer(vkb);

    dispose(std::move(vkb));

    const auto t1 = std::chrono::steady_clock::now();
    const auto frameTime = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);
    perf.frameTime.update(frameTime);
}

void Application::quitToMenu() {
    status.message = "Exiting...";
    status.value = 1.0f;

    NEXT(shutdownViews());
}

void Application::shutdownViews() {
    views->setCurrent(nullptr);
    views->clear();

    NEXT(shutdownClientSide());
}

void Application::stopClientSide() {
    views.reset();
    client.reset();
    renderer.reset();
    rendererBackground.reset();
    renderResources.reset();
    voxelShapeCache.reset();
}

void Application::stopServerSide() {
    if (worker) {
        udpClient->stop();
        worker->stop();
        udpClient.reset();
        worker.reset();
    }

    server.reset();
    assetsManager.reset();
}

void Application::shutdownClientSide() {
    stopClientSide();
    NEXT(shutdownServerSide());
}

void Application::shutdownServerSide() {
    stopServerSide();
    NEXT(shutdownDone());
}

void Application::shutdownDone() {
    status.message.clear();
    status.value = 0.0f;

    gui.mainMenu->setEnabled(true);
}

void Application::renderVersion(const Vector2i& viewport) {
    (void)viewport;

    static const Color4 color{Colors::text * alpha(0.5f)};
    const Vector2 pos{5.0f, 5.0f + static_cast<float>(config.guiFontSize)};
    canvas.drawText(pos, GAME_VERSION, font, config.guiFontSize, color);
}

void Application::renderFrameTime(const Vector2i& viewport) {
    static const Color4 color{Colors::text * alpha(0.5f)};

    const auto posX = static_cast<float>(viewport.x) - 450.0f;
    const auto fontSize = static_cast<float>(config.guiFontSize);

    {
        const auto renderTimeMs = static_cast<float>(perf.renderTime.value().count()) / 1000000.0f;
        const auto text = fmt::format("Render: {:.1f}ms", renderTimeMs);
        canvas.drawText({posX - 170.0f, 5.0f + fontSize}, text, font, config.guiFontSize, color);
    }

    {
        const auto frameTimeMs = static_cast<float>(perf.frameTime.value().count()) / 1000000.0f;
        const auto text = fmt::format("Frame: {:.1f}ms", frameTimeMs);
        canvas.drawText({posX - 170.0f, 5.0f + fontSize * 2.0}, text, font, config.guiFontSize, color);
    }

    {
        const auto vRamMB = static_cast<float>(getAllocator().getUsedBytes()) / 1048576.0f;
        const auto text = fmt::format("VRAM: {:.0f}MB", vRamMB);
        canvas.drawText({posX - 170.0f, 5.0f + fontSize * 3.0}, text, font, config.guiFontSize, color);
    }

    if (server) {
        const auto tickTimeMs = static_cast<float>(server->getPerfTickTime().count()) / 1000000.0f;
        const auto text = fmt::format("Tick: {:.1f}ms", tickTimeMs);
        canvas.drawText({posX - 170.0f, 5.0f + fontSize * 4.0}, text, font, config.guiFontSize, color);
    }
}

void Application::renderStatus(const Vector2i& viewport) {
    const auto fontHeight = static_cast<float>(config.guiFontSize) * 2.0f;
    canvas.drawText(Vector2{50.0f, static_cast<float>(viewport.y) - 75.0f - fontHeight},
                    status.message,
                    font,
                    config.guiFontSize * 2,
                    Colors::text);

    canvas.drawRect(Vector2{50.0f, static_cast<float>(viewport.y) - 75.0f},
                    {static_cast<float>(viewport.x) - 100.0f, 25.0f},
                    Colors::background);

    canvas.drawRect(Vector2{50.0f, static_cast<float>(viewport.y) - 75.0f},
                    {(static_cast<float>(viewport.x) - 100.0f) * status.value, 25.0f},
                    Colors::primary);
}

void Application::renderBanner(const Vector2i& viewport) {
    auto adjustedViewport = viewport - Vector2i{0, 200.0f};
    const auto size = bannerTexture.get().getSize2D();
    const auto pos = Vector2{adjustedViewport / 2 - size / 2};
    canvas.drawTexture(pos, size, bannerTexture.get(), Color4{1.0f});
}

void Application::loadProfile() {
    const auto profilePath = config.userdataPath / profileFilename;

    if (Fs::exists(profilePath)) {
        logger.info("Loading profile from: '{}'", profilePath);
        Xml::fromFile(profilePath, playerLocalProfile);
        gui.mainMenu->setEnabled(true);
    } else {
        logger.warn("No such profile found: '{}'", profilePath);
        gui.createProfile->setEnabled(true);
    }
}

void Application::createEditor() {
    status.message = "Entering...";
    status.value = 1.0f;
    // editor = std::make_unique<Editor>(config, *this, audio, *assetsManager, *voxelShapeCache, font);

    logger.info("Creating editor views");

    view.editor = views->addView<ViewBuild>(audio, *assetsManager, *voxelShapeCache, font);
    views->setCurrent(view.editor);
}

void Application::checkForClientScene() {
    status.message = "Entering...";
    status.value = 1.0f;

    if (client->getScene()) {
        logger.info("Client has a scene, creating game views");

        view.space = views->addView<ViewSpace>(*assetsManager, *voxelShapeCache, font, *client);
        view.galaxy = views->addView<ViewGalaxy>(*assetsManager, *voxelShapeCache, font, *client);
        view.system = views->addView<ViewSystem>(*assetsManager, *voxelShapeCache, font, *client);
        views->setCurrent(view.space);
    } else {
        NEXT(checkForClientScene());
    }
}

void Application::startClient() {
    logger.info("Starting client");

    status.message = "Connecting...";
    status.value = 0.9f;

    future = std::async([this]() -> std::function<void()> {
        client = std::make_unique<Client>(
            config, *assetsManager, playerLocalProfile, voxelShapeCache.get(), connectAddress, server->getPort());

        return [this]() { checkForClientScene(); };
    });
}

void Application::startServer() {
    logger.info("Starting server");

    status.message = "Starting universe (this may take a while)...";
    status.value = 0.8f;

    future = std::async([this]() -> std::function<void()> {
        auto* m = gui.loadSave->getMode() == MultiplayerMode::Online ? &matchmakerClient : nullptr;
        server = std::make_unique<Server>(config, *assetsManager, serverOptions, m);

        return [this]() { startClient(); };
    });
}

void Application::createEmptyThumbnail(RendererThumbnail& thumbnailRenderer) {
    logger.info("Creating empty thumbnail");

    thumbnailRenderer.render(nullptr, VoxelShape::Cube);
    const auto alloc =
        assetsManager->getImageAtlas().add(thumbnailRenderer.getViewport(), thumbnailRenderer.getFinalBuffer());
    assetsManager->addImage("block_empty_image", alloc);
}

void Application::createPlanetLowResTextures(RendererPlanetSurface& renderer) {
    uint64_t seed = 1;
    for (auto& planet : assetsManager->getPlanetTypes().findAll()) {
        auto textures = renderer.renderPlanet(seed++ * 100, planet);
        planet->setLowResTextures(std::move(textures));
    }
}

void Application::createPlanetLowResTextures() {
    logger.info("Creating planet low-res textures");

    status.message = "Creating planets...";
    status.value = 0.5f;

    const Vector2i viewport{config.graphics.planetLowResTextureSize, config.graphics.planetLowResTextureSize};
    auto rendererPlanetSurfaceLowRes =
        std::make_unique<RendererPlanetSurface>(config, viewport, *this, *renderResources, *voxelShapeCache);

    createPlanetLowResTextures(*rendererPlanetSurfaceLowRes);

    NEXT(createThumbnails());
}

void Application::createPlanetThumbnails(RendererThumbnail& thumbnailRenderer) {
    logger.info("Creating planet thumbnails");

    for (const auto& planet : assetsManager->getPlanetTypes().findAll()) {
        thumbnailRenderer.render(planet);
        const auto alloc =
            assetsManager->getImageAtlas().add(thumbnailRenderer.getViewport(), thumbnailRenderer.getFinalBuffer());

        const auto name = fmt::format("{}_image", planet->getName());
        planet->setThumbnail(assetsManager->addImage(name, alloc));
    }
}

void Application::createBlockThumbnails(RendererThumbnail& thumbnailRenderer) {
    logger.info("Creating block thumbnails");

    for (const auto& block : assetsManager->getBlocks().findAll()) {
        for (const auto shape : block->getShapes()) {
            thumbnailRenderer.render(block, shape);
            const auto alloc =
                assetsManager->getImageAtlas().add(thumbnailRenderer.getViewport(), thumbnailRenderer.getFinalBuffer());

            const auto name = fmt::format("{}_{}_image", block->getName(), VoxelShape::typeNames[shape]);
            block->setThumbnail(shape, assetsManager->addImage(name, alloc));
        }
    }
}

void Application::createThumbnails() {
    logger.info("Creating thumbnails");

    status.message = "Creating thumbnails...";
    status.value = 0.6f;

    auto rendererThumbnail = std::make_unique<RendererThumbnail>(config, *this, *renderResources, *voxelShapeCache);

    createBlockThumbnails(*rendererThumbnail);
    createEmptyThumbnail(*rendererThumbnail);
    createPlanetThumbnails(*rendererThumbnail);

    assetsManager->finalize();

    views = std::make_unique<ViewContext>(config, *this, *assetsManager, guiManager, *rendererBackground);

    if (editorOnly) {
        NEXT(createEditor());
    } else {
        NEXT(startServer());
    }
}

void Application::loadNextAssetInQueue(AssetsManager::LoadQueue::const_iterator next) {
    if (next == assetsManager->getLoadQueue().cend()) {
        NEXT(createRenderers());
    } else {
        const auto start = std::chrono::steady_clock::now();

        while (next != assetsManager->getLoadQueue().cend()) {
            const auto count = std::distance(assetsManager->getLoadQueue().cbegin(), next) + 1;
            const auto progress = static_cast<float>(count) / static_cast<float>(assetsManager->getLoadQueue().size());

            try {
                (*next)(this, &audio);
                ++next;
            } catch (...) {
                EXCEPTION_NESTED("Failed to load asset");
            }

            status.message = fmt::format("Loading assets ({}/{})...", count, assetsManager->getLoadQueue().size());
            status.value = 0.3f + progress * 0.2f;

            const auto now = std::chrono::steady_clock::now();
            const auto test = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
            if (test > std::chrono::microseconds(10000)) {
                break;
            }
        }

        NEXT(loadNextAssetInQueue(next));
    }
}

void Application::loadAssets() {
    logger.info("Loading assets");

    this->assetsManager->init(*this);

    status.message = "Loading assets...";
    status.value = 0.3f;

    loadNextAssetInQueue(assetsManager->getLoadQueue().cbegin());
}

void Application::createRegistry() {
    logger.info("Setting up assetsManager");

    status.message = "Loading mod packs...";
    status.value = 0.3f;

    future = std::async([this]() -> std::function<void()> {
        this->assetsManager = std::make_unique<AssetsManager>(config);
        return [this]() { loadAssets(); };
    });
}

void Application::createSceneRenderer(const Vector2i& viewport) {
    RenderOptions renderOptions{};
    renderOptions.viewport = viewport;
    renderOptions.shadowsSize = config.graphics.shadowsSize;
    renderOptions.shadowsLevel = config.graphics.shadowsLevel;
    renderOptions.ssao = config.graphics.ssao;
    renderOptions.bloom = config.graphics.bloom;
    renderOptions.fxaa = config.graphics.fxaa;
    renderer = std::make_unique<RendererScenePbr>(renderOptions, *this, *renderResources);
}

void Application::createRenderers() {
    logger.info("Creating renderer");

    status.message = "Creating renderer...";
    status.value = 0.5f;

    renderResources = std::make_unique<RenderResources>(
        *this, assetsManager->getBlockMaterialsUbo(), assetsManager->getMaterialTextures(), font, config.guiFontSize);

    createSceneRenderer(config.graphics.windowSize);

    rendererBackground = std::make_unique<RendererBackground>(config, *this, *renderResources, *voxelShapeCache);

    NEXT(createPlanetLowResTextures());
}

void Application::createVoxelShapeCache() {
    logger.info("Creating voxel shape cache");

    status.message = "Creating voxel shape cache...";
    status.value = 0.2f;

    voxelShapeCache = std::make_unique<VoxelShapeCache>(config);

    NEXT(createRegistry());
}

void Application::compressAssets() {
    logger.info("Compressing assets");

    status.message = "Compressing assets (may take several minutes)...";
    status.value = 0.1f;

    future = std::async([this]() -> std::function<void()> {
        AssetsManager::compressAssets(config);
        return [=]() { createVoxelShapeCache(); };
    });
}

void Application::checkOnlineServices() {
    gui.logIn = guiManager.addWindow<GuiWindowLogIn>(*this, matchmakerClient);
    guiManager.showModal(*gui.logIn);
    gui.logIn->start();

    gui.logIn->setOnClose([this]() {
        guiManager.closeModal(*gui.logIn);
        gui.mainMenu->setEnabled(true);
    });

    gui.logIn->setOnSuccessCallback([this]() {
        matchmakerClient.saveDataFile();
        guiManager.closeModal(*gui.logIn);
        gui.serverBrowser->setEnabled(true);
        gui.serverBrowser->fetchServers(1);
    });
}

void Application::startMultiPlayerHosted() {
    config.network.clientBindAddress = "::1";
    config.network.serverBindAddress = "::";
    connectAddress = "::1";

    logger.info("Starting multi player mode");

    status.message = "Loading...";
    status.value = 0.0f;

    // gui.mainMenu.setEnabled(false);

    NEXT(compressAssets());
}

void Application::startSinglePlayer() {
    config.network.clientBindAddress = "::1";
    config.network.serverBindAddress = "::1";
    connectAddress = "::1";

    logger.info("Starting single player mode");

    status.message = "Loading...";
    status.value = 0.0f;

    // gui.mainMenu.setEnabled(false);

    NEXT(compressAssets());
}

void Application::openMultiplayerSettings() {
    gui.multiplayerSettings->setEnabled(true);
    gui.multiplayerSettings->setServerName(fmt::format("{} server", playerLocalProfile.name));
}

void Application::startConnectServer(const std::string& serverId) {
    /*future = std::async([this, address, port]() -> std::function<void()> {
        worker = std::make_unique<BackgroundWorker>();
        udpClient = std::make_unique<NetworkUdpClient>(config, worker->getService());
        udpClient->connect(address, port);
        return [=]() {};
    });*/

    logger.info("Starting connection to the server: {}", serverId);
    worker = std::make_unique<BackgroundWorker>();
    // udpClient = std::make_shared<NetworkUdpClient>(config, worker->getService());

    logger.info("Sending STUN request");
    /*udpClient->getStunClient().send([this, serverId](const NetworkStunClient::Result& result) {
        Matchmaker::ServerConnectModel body{};
        body.address = result.endpoint.address().to_string();
        body.port = result.endpoint.port();

        logger.info("Sending connection request to the matchmaker server");
        matchmaker->apiServersConnect(serverId, body, [this](const Matchmaker::ServerConnectResponse& res) {
            if (!res.error.empty()) {
                logger.error("Connection request failed with error: {}", res.error);
            } else {
                logger.info("Got response from the matchmaker server");

                future = std::async([this, endpoint = res.data]() -> std::function<void()> {
                    udpClient->connect(endpoint.address, endpoint.port);

                    return []() {};
                });
            }
        });
    });*/
}

void Application::startEditor() {
    logger.info("Starting editor mode");
    editorOnly = true;
    startSinglePlayer();
}

bool Application::isViewsInputSuspended() const {
    return gui.gameMenu->isEnabled() || gui.settings->isEnabled();
}

void Application::eventMouseMoved(const Vector2i& pos) {
    mousePos = pos;
    guiManager.eventMouseMoved(pos);
    if (views && !guiManager.isMousePosOverlap(mousePos) && !isViewsInputSuspended()) {
        views->eventMouseMoved(pos);
    }
    /*if (!nuklear.isCursorInsideWindow(pos)) {
        if (game) {
            game->eventMouseMoved(pos);
        } else if (editor) {
            editor->eventMouseMoved(pos);
        }
    }*/
}

void Application::eventMousePressed(const Vector2i& pos, MouseButton button) {
    mousePos = pos;
    guiManager.eventMousePressed(pos, button);
    if (views && !guiManager.isMousePosOverlap(mousePos) && !isViewsInputSuspended()) {
        views->eventMousePressed(pos, button);
    }
    /*if (!nuklear.isCursorInsideWindow(pos) && !nuklear.isInputActive()) {
        if (game) {
            game->eventMousePressed(pos, button);
        } else if (editor) {
            editor->eventMousePressed(pos, button);
        }
    }*/
}

void Application::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    mousePos = pos;
    guiManager.eventMouseReleased(pos, button);
    if (views && !isViewsInputSuspended()) {
        views->eventMouseReleased(pos, button);
    }
    /*if (!nuklear.isInputActive()) {
        if (game) {
            game->eventMouseReleased(pos, button);
        } else if (editor) {
            editor->eventMouseReleased(pos, button);
        }
    }*/
}

void Application::eventMouseScroll(const int xscroll, const int yscroll) {
    guiManager.eventMouseScroll(xscroll, yscroll);
    if (views && !guiManager.isMousePosOverlap(mousePos) && !isViewsInputSuspended()) {
        views->eventMouseScroll(xscroll, yscroll);
    }
    /*if (!nuklear.isCursorInsideWindow(mousePos) && !nuklear.isInputActive()) {
        if (game) {
            game->eventMouseScroll(xscroll, yscroll);
        } else if (editor) {
            editor->eventMouseScroll(xscroll, yscroll);
        }
    }*/
}

void Application::eventKeyPressed(const Key key, const Modifiers modifiers) {
    guiManager.eventKeyPressed(key, modifiers);

    if (modifiers == 0 && key == Key::LetterM && !view.editor && !isViewsInputSuspended()) {
        if (views->getCurrent() == view.galaxy) {
            views->setCurrent(view.space);
        } else {
            views->setCurrent(view.galaxy);
        }
    } else if (modifiers == 0 && key == Key::LetterN && !view.editor && !isViewsInputSuspended()) {
        if (views->getCurrent() == view.system) {
            views->setCurrent(view.space);
        } else {
            views->setCurrent(view.system);
        }
    } else if (views && key == Key::Escape) {
        if (gui.gameMenu->isEnabled()) {
            gui.gameMenu->close();
        } else if (gui.settings->isEnabled()) {
            gui.settings->close();
            gui.gameMenu->setEnabled(true);
            guiManager.setFocused(*gui.gameMenu);
        } else {
            gui.gameMenu->setEnabled(true);
            guiManager.setFocused(*gui.gameMenu);
        }
    } else if (views && !isViewsInputSuspended()) {
        views->eventKeyPressed(key, modifiers);
    }
}
/*if (!nuklear.isInputActive()) {
    if (game) {
        game->eventKeyPressed(key, modifiers);
    } else if (editor) {
        editor->eventKeyPressed(key, modifiers);
    }
}*/

void Application::eventKeyReleased(const Key key, const Modifiers modifiers) {
    guiManager.eventKeyReleased(key, modifiers);
    if (views && !isViewsInputSuspended()) {
        views->eventKeyReleased(key, modifiers);
    }
    /*if (!nuklear.isInputActive()) {
        if (game) {
            game->eventKeyReleased(key, modifiers);
        } else if (editor) {
            editor->eventKeyReleased(key, modifiers);
        }
    }*/
}

void Application::eventWindowResized(const Vector2i& size) {
    waitDeviceIdle();
}

void Application::eventCharTyped(const uint32_t code) {
    guiManager.eventCharTyped(code);
    if (views && !isViewsInputSuspended()) {
        views->eventCharTyped(code);
    }

    /*if (!nuklear.isInputActive()) {
        if (game) {
            game->eventCharTyped(code);
        } else if (editor) {
            editor->eventCharTyped(code);
        }
    }*/
}

void Application::eventWindowInputBegin() {
    guiManager.eventInputBegin();
}

void Application::eventWindowInputEnd() {
    guiManager.eventInputEnd();
}

void Application::eventWindowBlur() {
}

void Application::eventWindowFocus() {
}
