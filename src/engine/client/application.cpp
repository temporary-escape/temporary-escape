#include "application.hpp"
#include "../graphics/theme.hpp"
#include "../utils/random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);
static const auto profileFilename = "profile.yaml";

Application::Application(const Config& config) :
    VulkanRenderer{config},
    config{config},
    canvas{*this},
    font{*this, config.fontsPath, config.guiFontName, config.guiFontSize * 2.0f},
    nuklear{config, canvas, font, config.guiFontSize},
    audio{} {

    gui.mainMenu.setItems({
        {"Singleplayer", [this]() { startSinglePlayer(); }},
        {"Multiplayer", []() {}},
        {"Settings", []() {}},
        {"Editor", [this]() { startEditor(); }},
        {"Mods", []() {}},
        {"Exit", [this]() { closeWindow(); }},
    });
    gui.mainMenu.setFontSize(config.guiFontSize * 1.25f);

    gui.createProfile.setOnSuccess([this](const GuiCreateProfile::Form& form) {
        playerLocalProfile.name = form.name;
        playerLocalProfile.secret = randomId();
        playerLocalProfile.toYaml(this->config.userdataPath / profileFilename);

        gui.mainMenu.setEnabled(true);
    });

    if (Fs::exists(config.userdataPath / profileFilename)) {
        playerLocalProfile.fromYaml(config.userdataPath / profileFilename);
        gui.createProfile.setEnabled(false);
    } else {
        gui.mainMenu.setEnabled(false);
    }

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

void Application::render(const Vector2i& viewport, const float deltaTime) {
    const auto t0 = std::chrono::steady_clock::now();

    if (future.valid() && future.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {
        auto fn = future.get();
        fn();
    }

    if (client) {
        client->update();
    }

    auto vkb = createCommandBuffer();

    if (renderer) {
        renderer->update();
    }

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

    if (game) {
        game->update(deltaTime);
        game->render(vkb, viewport);
    } else if (editor) {
        editor->update(deltaTime);
        editor->render(vkb, viewport);
    }

    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getSwapChainFramebuffer();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValues = {clearColor};

    vkb.beginRenderPass(renderPassInfo);

    if ((game && game->isReady()) || editor) {
        renderer->blit(vkb, viewport);
    }

    canvas.begin(viewport);

    renderVersion(viewport);
    renderFrameTime(viewport);

    if (game && game->isReady()) {
        game->renderCanvas(canvas, nuklear, viewport);
    } else if (editor) {
        editor->renderCanvas(canvas, nuklear, viewport);
    } else {
        if (!status.message.empty()) {
            renderStatus(viewport);
        }

        nuklear.begin(viewport);
        nuklear.draw(gui.mainMenu);
        nuklear.draw(gui.createProfile);
        nuklear.end();
    }

    canvas.end(vkb);

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
    canvas.font(font.regular, config.guiFontSize);
    canvas.text({5.0f, 5.0f + config.guiFontSize}, GAME_VERSION);
}

void Application::renderFrameTime(const Vector2i& viewport) {
    canvas.color(Theme::text * alpha(0.5f));
    canvas.font(font.regular, config.guiFontSize);

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

    if (server) {
        const auto tickTimeMs = static_cast<float>(server->getPerfTickTime().count()) / 1000000.0f;
        const auto text = fmt::format("Tick: {:.1f}ms", tickTimeMs);
        canvas.text({viewport.x - 170.0f, 5.0f + config.guiFontSize * 4.0}, text);
    }
}

void Application::renderStatus(const Vector2i& viewport) {
    canvas.color(Theme::text);
    canvas.font(font.regular, config.guiFontSize);
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
    editor = std::make_unique<Editor>(config, *renderer, *assetsManager, font);
}

void Application::checkForClientScene() {
    status.message = "Entering...";
    status.value = 1.0f;

    if (client->getScene()) {
        logger.info("Client has a scene, creating Game instance");

        game = std::make_unique<Game>(
            config, *renderer, *skyboxGenerator, *planetGenerator, *assetsManager, font, *client);
    } else {
        NEXT(checkForClientScene());
    }
}

void Application::startClient() {
    logger.info("Starting client");

    status.message = "Connecting...";
    status.value = 0.9f;

    client = std::make_unique<Client>(config, *assetsManager, playerLocalProfile);

    logger.info("Connecting to the server");

    future = std::async([this]() -> std::function<void()> {
        client->connect("localhost", config.serverPort);

        return [this]() { checkForClientScene(); };
    });
}

void Application::startServer() {
    logger.info("Starting server");

    status.message = "Starting universe (this may take a while)...";
    status.value = 0.8f;

    future = std::async([this]() -> std::function<void()> {
        serverCerts = std::make_unique<Server::Certs>();
        server = std::make_unique<Server>(config, *serverCerts, *assetsManager, *db);

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

void Application::createEmptyThumbnail(Renderer& thumbnailRenderer) {
    logger.info("Creating empty thumbnail");

    thumbnailRenderer.render(nullptr, VoxelShape::Cube);
    const auto alloc =
        assetsManager->getImageAtlas().add(thumbnailRenderer.getViewport(), thumbnailRenderer.getTexture());
    assetsManager->addImage("block_empty_image", alloc);
}

void Application::createPlanetLowResTextures(PlanetGenerator& planetGenerator) {
    uint64_t seed = 1;
    for (auto& planet : assetsManager->getPlanetTypes().findAll()) {
        auto textures = planetGenerator.render(seed++ * 100, planet);
        planet->setLowResTextures(std::move(textures));
    }
}

void Application::createPlanetLowResTextures() {
    logger.info("Creating planet low-res textures");

    status.message = "Creating planets...";
    status.value = 0.5f;

    const auto planetTextureSize = Vector2i{config.graphics.planetLowResTextureSize};
    auto planetGeneratorLowRes =
        std::make_unique<PlanetGenerator>(planetTextureSize, *this, *renderResources, *assetsManager);

    createPlanetLowResTextures(*planetGeneratorLowRes);

    NEXT(createThumbnails());
}

void Application::createPlanetThumbnails(Renderer& thumbnailRenderer) {
    logger.info("Creating planet thumbnails");

    for (const auto& planet : assetsManager->getPlanetTypes().findAll()) {
        thumbnailRenderer.render(planet);
        const auto alloc =
            assetsManager->getImageAtlas().add(thumbnailRenderer.getViewport(), thumbnailRenderer.getTexture());

        const auto name = fmt::format("{}_image", planet->getName());
        planet->setThumbnail(assetsManager->addImage(name, alloc));
    }
}

void Application::createBlockThumbnails(Renderer& thumbnailRenderer) {
    logger.info("Creating block thumbnails");

    for (const auto& block : assetsManager->getBlocks().findAll()) {
        for (const auto shape : block->getShapes()) {
            thumbnailRenderer.render(block, shape);
            const auto alloc =
                assetsManager->getImageAtlas().add(thumbnailRenderer.getViewport(), thumbnailRenderer.getTexture());

            const auto name = fmt::format("{}_{}_image", block->getName(), VoxelShape::typeNames[shape]);
            block->setThumbnail(shape, assetsManager->addImage(name, alloc));
        }
    }
}

void Application::createThumbnails() {
    logger.info("Creating thumbnails");

    status.message = "Creating thumbnails...";
    status.value = 0.6f;

    const auto viewport = Vector2i{config.thumbnailSize, config.thumbnailSize};
    auto thumbnailRenderer = std::make_unique<Renderer>(
        config, viewport, *this, *renderResources, canvas, nuklear, *voxelShapeCache, *assetsManager, font);

    createBlockThumbnails(*thumbnailRenderer);
    createEmptyThumbnail(*thumbnailRenderer);
    createPlanetThumbnails(*thumbnailRenderer);

    assetsManager->finalize();

    if (editorOnly) {
        NEXT(createEditor());
    } else {
        NEXT(startDatabase());
    }
}

void Application::loadNextAssetInQueue(AssetsManager::LoadQueue::const_iterator next) {
    if (next == assetsManager->getLoadQueue().cend()) {
        NEXT(createRenderer());
    } else {
        const auto start = std::chrono::steady_clock::now();

        while (next != assetsManager->getLoadQueue().cend()) {
            const auto count = std::distance(assetsManager->getLoadQueue().cbegin(), next) + 1;
            const auto progress = static_cast<float>(count) / static_cast<float>(assetsManager->getLoadQueue().size());

            try {
                (*next)(*this);
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

void Application::createRenderer() {
    logger.info("Creating renderer");

    status.message = "Creating renderer...";
    status.value = 0.5f;

    const auto viewport = Vector2i{config.graphics.windowWidth, config.graphics.windowHeight};
    renderResources = std::make_unique<RenderResources>(*this);
    renderer = std::make_unique<Renderer>(
        config, viewport, *this, *renderResources, canvas, nuklear, *voxelShapeCache, *assetsManager, font);

    skyboxGenerator = std::make_unique<SkyboxGenerator>(config, *this, *renderResources, *assetsManager);

    const Vector2i planetTextureSize{config.graphics.planetTextureSize, config.graphics.planetTextureSize};
    planetGenerator = std::make_unique<PlanetGenerator>(planetTextureSize, *this, *renderResources, *assetsManager);

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

    gui.mainMenu.setEnabled(false);

    NEXT(compressAssets());

    // game = std::make_unique<Game>(config, *this, canvas, font, nuklear, skyboxGenerator);
    /*future = std::async([]() {
        assetsManager = std::make_unique<Registry>(config);
    });*/
}

void Application::startEditor() {
    logger.info("Starting editor mode");
    editorOnly = true;
    startSinglePlayer();
}

void Application::eventMouseMoved(const Vector2i& pos) {
    mousePos = pos;
    nuklear.eventMouseMoved(pos);
    if (!nuklear.isCursorInsideWindow(pos)) {
        if (game) {
            game->eventMouseMoved(pos);
        } else if (editor) {
            editor->eventMouseMoved(pos);
        }
    }
}

void Application::eventMousePressed(const Vector2i& pos, MouseButton button) {
    mousePos = pos;
    nuklear.eventMousePressed(pos, button);
    if (!nuklear.isCursorInsideWindow(pos) && !nuklear.isInputActive()) {
        if (game) {
            game->eventMousePressed(pos, button);
        } else if (editor) {
            editor->eventMousePressed(pos, button);
        }
    }
}

void Application::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    mousePos = pos;
    nuklear.eventMouseReleased(pos, button);
    if (!nuklear.isInputActive()) {
        if (game) {
            game->eventMouseReleased(pos, button);
        } else if (editor) {
            editor->eventMouseReleased(pos, button);
        }
    }
}

void Application::eventMouseScroll(const int xscroll, const int yscroll) {
    nuklear.eventMouseScroll(xscroll, yscroll);
    if (!nuklear.isCursorInsideWindow(mousePos) && !nuklear.isInputActive()) {
        if (game) {
            game->eventMouseScroll(xscroll, yscroll);
        } else if (editor) {
            editor->eventMouseScroll(xscroll, yscroll);
        }
    }
}

void Application::eventKeyPressed(const Key key, const Modifiers modifiers) {
    nuklear.eventKeyPressed(key, modifiers);
    if (!nuklear.isInputActive()) {
        if (game) {
            game->eventKeyPressed(key, modifiers);
        } else if (editor) {
            editor->eventKeyPressed(key, modifiers);
        }
    }
}

void Application::eventKeyReleased(const Key key, const Modifiers modifiers) {
    nuklear.eventKeyReleased(key, modifiers);
    if (!nuklear.isInputActive()) {
        if (game) {
            game->eventKeyReleased(key, modifiers);
        } else if (editor) {
            editor->eventKeyReleased(key, modifiers);
        }
    }
}

void Application::eventWindowResized(const Vector2i& size) {
}

void Application::eventCharTyped(const uint32_t code) {
    nuklear.eventCharTyped(code);
    if (!nuklear.isInputActive()) {
        if (game) {
            game->eventCharTyped(code);
        } else if (editor) {
            editor->eventCharTyped(code);
        }
    }
}

void Application::eventWindowBlur() {
}

void Application::eventWindowFocus() {
}
