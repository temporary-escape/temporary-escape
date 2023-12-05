#include "Application.hpp"
#include "../File/OggFileReader.hpp"
#include "../File/PngFileReader.hpp"
#include "../Graphics/Theme.hpp"
#include "../Utils/Random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);
static const auto profileFilename = "profile.xml";

Application::Application(Config& config) :
    VulkanRenderer{config},
    config{config},
    audio{},
    audioSource{audio.createSource()},
    canvas{*this},
    font{*this, config.fontsPath, config.guiFontName, config.guiFontSize * 2.0f},
    nuklear{*this, config, canvas, font, config.guiFontSize},
    rendererCanvas{*this},
    canvas2{*this},
    guiManager{*this, rendererCanvas, font, config.guiFontSize} {

    loadSounds();

    const std::string gpuName{getPhysicalDeviceProperties().deviceName};
    if (gpuName.find("UHD Graphics") != std::string::npos) {
        config.graphics.planetTextureSize = 512;
        config.graphics.skyboxSize = 1024;
        config.graphics.ssao = false;
    }

    guiManager.addWindow<GuiWindowMainMenu>();

    /*gui.mainMenu.setItems({
        {"Singleplayer", [this]() { startSinglePlayer(); }},
        {"Multiplayer", []() {}},
        {"Settings",
         [this]() {
             gui.mainSettings.setSettings(this->config);
             gui.mainSettings.setVideoModes(getSupportedResolutionModes());
             gui.mainSettings.setEnabled(true);
         }},
        {"Editor", [this]() { startEditor(); }},
        {"Mods", []() {}},
        {"Exit", [this]() { closeWindow(); }},
    });
    gui.mainMenu.setFontSize(config.guiFontSize * 1.25f);*/

    /*gui.createProfile.setOnSuccess([this](const GuiCreateProfile::Form& form) {
        playerLocalProfile.name = form.name;
        playerLocalProfile.secret = randomId();
        Xml::toFile(this->config.userdataPath / profileFilename, playerLocalProfile);

        gui.mainMenu.setEnabled(true);
    });*/

    if (Fs::exists(config.userdataPath / profileFilename)) {
        Xml::fromFile(config.userdataPath / profileFilename, playerLocalProfile);
        // gui.createProfile.setEnabled(false);
    } else {
        // gui.mainMenu.setEnabled(false);
    }

    /*gui.keepSettings.setEnabled(false);
    gui.mainSettings.setEnabled(false);
    gui.mainSettings.setOnApply([this](const Config& updated) {
        logger.info("Updated config");
        this->setWindowFullScreen(updated.graphics.fullscreen);
        this->setWindowResolution({updated.graphics.windowWidth, updated.graphics.windowHeight});
        this->gui.keepSettings.setEnabled(true);
        this->gui.keepSettings.reset();
        this->gui.keepSettings.setOnResult([this, updated](bool result) {
            if (result) {
                this->config = updated;
                Xml::toFile(this->config.userdataPath / "settings.xml", this->config);
            } else {
                this->setWindowFullScreen(this->config.graphics.fullscreen);
                this->setWindowResolution({this->config.graphics.windowWidth, this->config.graphics.windowHeight});
            }
        });
    });*/

    VkQueryPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    createInfo.queryCount = MAX_FRAMES_IN_FLIGHT;
    renderQueryPool = createQueryPool(createInfo);
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
}

bool Application::shouldBlit() const {
    return shouldBlitCount <= 0;
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
        client->update();
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

    if (renderer) {
        renderer->setMousePos(mousePos);
    }

    if (game) {
        game->update(deltaTime);
        game->render(vkb, *renderer, viewport);
    } else if (editor) {
        editor->update(deltaTime);
        editor->render(vkb, *renderer, viewport);
    }

    if ((!game || !game->isReady()) && !editor) {
        shouldBlitCount = 2;
    } else {
        --shouldBlitCount;
    }

    // GUI
    guiManager.render(vkb, viewport);

    // HUD
    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getSwapChainFramebuffer();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValues = {clearColor};

    if (shouldBlit() && renderer->isBlitReady()) {
        renderer->transitionForBlit(vkb);
    }

    vkb.beginRenderPass(renderPassInfo);
    vkb.setViewport({0, 0}, viewport);
    vkb.setScissor({0, 0}, viewport);

    if (shouldBlit() && renderer->isBlitReady()) {
        renderer->blit(vkb);
    }

    canvas.begin(viewport);

    renderVersion(viewport);
    renderFrameTime(viewport);

    if (shouldBlit()) {
        if (game && game->isReady()) {
            game->renderCanvas(canvas, nuklear, viewport);
        } else if (editor) {
            editor->renderCanvas(canvas, nuklear, viewport);
        }
    }

    if (!shouldBlit()) {
        if (!status.message.empty()) {
            renderStatus(viewport);
        }

        /*nuklear.begin(viewport);
        gui.mainMenu.draw(nuklear, viewport);
        gui.createProfile.draw(nuklear, viewport);
        gui.mainSettings.draw(nuklear, viewport);
        gui.keepSettings.draw(nuklear, viewport);
        nuklear.end();*/

        /*canvas2.begin();
        canvas2.drawRect({200.0f, 200.0f}, {300.0f, 100.0f}, {1.0f, 0.0f, 0.0f, 1.0f});
        canvas2.drawRect({500.0f, 500.0f}, {50.0f, 100.0f}, {0.0f, 1.0f, 0.0f, 1.0f});
        canvas2.flush();

        rendererCanvas.render(vkb, canvas2, viewport);*/
    }

    canvas.end(vkb);

    canvas2.begin(viewport);
    guiManager.blit(canvas2);
    canvas2.flush();
    rendererCanvas.render(vkb, canvas2, viewport);

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

void Application::renderVersion(const Vector2i& viewport) {
    canvas.color(Theme::text * alpha(0.5f));
    canvas.font(font.get(FontFace::Regular), config.guiFontSize);
    canvas.text({5.0f, 5.0f + config.guiFontSize}, GAME_VERSION);
}

void Application::renderFrameTime(const Vector2i& viewport) {
    canvas.color(Theme::text * alpha(0.5f));
    canvas.font(font.get(FontFace::Regular), config.guiFontSize);

    {
        const auto renderTimeMs = static_cast<float>(perf.renderTime.value().count()) / 1000000.0f;
        const auto text = fmt::format("Render: {:.1f}ms", renderTimeMs);
        canvas.text({viewport.x - 170.0f, 5.0f + config.guiFontSize}, text);
    }

    {
        const auto frameTimeMs = static_cast<float>(perf.frameTime.value().count()) / 1000000.0f;
        const auto text = fmt::format("Frame: {:.1f}ms", frameTimeMs);
        canvas.text({viewport.x - 170.0f, 5.0f + config.guiFontSize * 2.0}, text);
    }

    {
        const auto vRamMB = static_cast<float>(getAllocator().getUsedBytes()) / 1048576.0f;
        const auto text = fmt::format("VRAM: {:.0f}MB", vRamMB);
        canvas.text({viewport.x - 170.0f, 5.0f + config.guiFontSize * 3.0}, text);
    }

    {
        const auto text = fmt::format("render: {}", shouldBlit());
        canvas.text({viewport.x - 170.0f, 5.0f + config.guiFontSize * 4.0}, text);
    }

    if (server) {
        const auto tickTimeMs = static_cast<float>(server->getPerfTickTime().count()) / 1000000.0f;
        const auto text = fmt::format("Tick: {:.1f}ms", tickTimeMs);
        canvas.text({viewport.x - 170.0f, 5.0f + config.guiFontSize * 5.0}, text);
    }
}

void Application::renderStatus(const Vector2i& viewport) {
    canvas.color(Theme::text);
    canvas.font(font.get(FontFace::Regular), config.guiFontSize);
    const auto fontHeight = static_cast<float>(config.guiFontSize) * 1.25f;
    canvas.text(Vector2{50.0f, static_cast<float>(viewport.y) - 75.0f - fontHeight}, status.message);

    canvas.color(Theme::backgroundTransparent);
    canvas.rect(Vector2{50.0f, static_cast<float>(viewport.y) - 75.0f},
                {static_cast<float>(viewport.x) - 100.0f, 25.0f});

    canvas.color(Theme::primary);
    canvas.rect(Vector2{50.0f, static_cast<float>(viewport.y) - 75.0f},
                {(static_cast<float>(viewport.x) - 100.0f) * status.value, 25.0f});
}

#define NEXT(expr)                                                                                                     \
    promise = decltype(promise){};                                                                                     \
    promise.set_value([=]() { (expr); });                                                                              \
    future = promise.get_future();

void Application::createEditor() {
    status.message = "Entering...";
    status.value = 1.0f;
    editor = std::make_unique<Editor>(config, *this, audio, *assetsManager, *voxelShapeCache, font);
}

void Application::checkForClientScene() {
    status.message = "Entering...";
    status.value = 1.0f;

    if (client->getScene()) {
        logger.info("Client has a scene, creating Game instance");

        game = std::make_unique<Game>(
            config, *this, *rendererSkybox, *rendererPlanetSurface, *assetsManager, *voxelShapeCache, font, *client);
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
            config, *assetsManager, playerLocalProfile, voxelShapeCache.get(), "localhost", config.serverPort);

        return [this]() { checkForClientScene(); };
    });
}

void Application::startServer() {
    logger.info("Starting server");

    status.message = "Starting universe (this may take a while)...";
    status.value = 0.8f;

    future = std::async([this]() -> std::function<void()> {
        server = std::make_unique<Server>(config, *assetsManager, *db);

        return [this]() { startClient(); };
    });
}

void Application::startDatabase() {
    logger.info("Starting database");

    status.message = "Starting world database...";
    status.value = 0.7f;

    future = std::async([this]() -> std::function<void()> {
        try {
            auto path = config.userdataSavesPath;
            if (config.saveFolderName) {
                path /= config.saveFolderName.value();
            } else {
                path /= "Universe 1";
            }
            if (config.saveFolderClean) {
                logger.warn("Deleting save: '{}'", path);
                Fs::remove_all(path);
            }
            if (!Fs::exists(path)) {
                logger.info("Creating save: '{}'", path);
                Fs::create_directories(path);
            }
            logger.info("Starting database with save: '{}'", path);

            DatabaseRocksDB::Options options{};
            options.cacheSizeMb = config.server.dbCacheSize;
            options.debugLogging = config.server.dbDebug;
            options.compression = config.server.dbCompression;
            db = std::make_unique<DatabaseRocksDB>(path, DatabaseRocksDB::Options{});
        } catch (...) {
            EXCEPTION_NESTED("Failed to start the database");
        }

        return [this]() { startServer(); };
    });
}

void Application::createEmptyThumbnail(RendererThumbnail& thumbnailRenderer) {
    logger.info("Creating empty thumbnail");

    thumbnailRenderer.render(nullptr, VoxelShape::Cube);
    const auto alloc =
        assetsManager->getImageAtlas().add(thumbnailRenderer.getViewport(), thumbnailRenderer.getFinalBuffer());
    assetsManager->addImage("block_empty_image", alloc);
}

void Application::createPlanetLowResTextures(RendererPlanetSurface& rendererPlanetSurface) {
    uint64_t seed = 1;
    for (auto& planet : assetsManager->getPlanetTypes().findAll()) {
        auto textures = rendererPlanetSurface.renderPlanet(seed++ * 100, planet);
        planet->setLowResTextures(std::move(textures));
    }
}

void Application::createPlanetLowResTextures() {
    logger.info("Creating planet low-res textures");

    status.message = "Creating planets...";
    status.value = 0.5f;

    const Vector2i viewport{config.graphics.planetLowResTextureSize, config.graphics.planetLowResTextureSize};
    auto rendererPlanetSurfaceLowRes = std::make_unique<RendererPlanetSurface>(
        config, viewport, *this, *renderResources, *assetsManager, *voxelShapeCache);

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

    auto rendererThumbnail =
        std::make_unique<RendererThumbnail>(config, *this, *renderResources, *assetsManager, *voxelShapeCache);

    createBlockThumbnails(*rendererThumbnail);
    createEmptyThumbnail(*rendererThumbnail);
    createPlanetThumbnails(*rendererThumbnail);

    assetsManager->finalize();

    if (editorOnly) {
        NEXT(createEditor());
    } else {
        NEXT(startDatabase());
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
    renderer = std::make_unique<RendererScenePbr>(renderOptions, *this, *renderResources, *assetsManager);
}

void Application::createRenderers() {
    logger.info("Creating renderer");

    status.message = "Creating renderer...";
    status.value = 0.5f;

    renderResources = std::make_unique<RenderResources>(*this);

    createSceneRenderer({config.graphics.windowWidth, config.graphics.windowHeight});

    rendererSkybox =
        std::make_unique<RendererSkybox>(config, *this, *renderResources, *assetsManager, *voxelShapeCache);

    const Vector2i viewport{config.graphics.planetTextureSize, config.graphics.planetTextureSize};
    rendererPlanetSurface = std::make_unique<RendererPlanetSurface>(
        config, viewport, *this, *renderResources, *assetsManager, *voxelShapeCache);

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

void Application::startSinglePlayer() {
    logger.info("Starting single player mode");

    status.message = "Loading...";
    status.value = 0.0f;

    // gui.mainMenu.setEnabled(false);

    NEXT(compressAssets());
}

void Application::startEditor() {
    logger.info("Starting editor mode");
    editorOnly = true;
    startSinglePlayer();
}

void Application::eventMouseMoved(const Vector2i& pos) {
    mousePos = pos;
    guiManager.eventMouseMoved(pos);
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
    /*if (!nuklear.isInputActive()) {
        if (game) {
            game->eventKeyPressed(key, modifiers);
        } else if (editor) {
            editor->eventKeyPressed(key, modifiers);
        }
    }*/
}

void Application::eventKeyReleased(const Key key, const Modifiers modifiers) {
    guiManager.eventKeyReleased(key, modifiers);
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
    /*if (!nuklear.isInputActive()) {
        if (game) {
            game->eventCharTyped(code);
        } else if (editor) {
            editor->eventCharTyped(code);
        }
    }*/
}

void Application::eventWindowBlur() {
}

void Application::eventWindowFocus() {
}

void Application::loadSounds() {
    const auto load = [this](AudioBuffer& buffer, const Path& path) {
        try {
            OggFileReader file{path};
            const auto freq = file.getFrequency();
            const auto format = file.getFormat();
            const auto data = file.readData();
            buffer = audio.createBuffer(data.data(), data.size(), format, freq);
        } catch (...) {
            EXCEPTION_NESTED("Failed to load sound: {}", path);
        }
    };

    load(sounds.uiClick, config.assetsPath / "base" / "sounds" / "ui_click_01.ogg");
}

void Application::nuklearOnClick(const bool push) {
    audioSource.bind(sounds.uiClick);
    audioSource.play();
}