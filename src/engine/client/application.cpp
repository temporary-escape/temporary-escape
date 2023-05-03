#include "application.hpp"
#include "../graphics/theme.hpp"
#include "../utils/random.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);
static const auto profileFilename = "profile.yaml";

Application::Application(const Config& config) :
    VulkanRenderer{config},
    config{config},
    canvas{*this},
    font{*this, config.fontsPath, config.guiFontName, config.guiFontSize * 2.0f},
    nuklear{config, canvas, font, config.guiFontSize} {

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
}

Application::~Application() {
    shouldStop.store(true);
    if (future.valid()) {
        future.get();
    }
}

void Application::render(const Vector2i& viewport, const float deltaTime) {
    if (future.valid() && future.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {
        auto fn = future.get();
        fn();
    }

    if (client) {
        client->update();
    }

    auto vkb = createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

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

    if (game || editor) {
        renderer->blit(vkb, viewport);
    }

    canvas.begin(viewport);

    if (game) {
        game->renderCanvas(canvas, nuklear, viewport);
    } else if (editor) {
        editor->renderCanvas(canvas, nuklear, viewport);
    } else {
        if (!game && !editor) {
            if (!status.message.empty()) {
                renderStatus(viewport);
            }
            renderVersion(viewport);
        }

        nuklear.begin(viewport);
        nuklear.draw(gui.mainMenu);
        nuklear.draw(gui.createProfile);
        nuklear.end();
    }

    canvas.end(vkb);

    vkb.endRenderPass();

    vkb.end();

    submitPresentCommandBuffer(vkb);

    dispose(std::move(vkb));
}

void Application::renderVersion(const Vector2i& viewport) {
    canvas.color(Theme::text * alpha(0.5f));
    canvas.font(font.regular, config.guiFontSize);
    canvas.text({5.0f, 5.0f + config.guiFontSize}, GAME_VERSION);
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
    editor = std::make_unique<Editor>(config, *renderer, *registry, font);
}

void Application::checkForClientScene() {
    status.message = "Entering...";
    status.value = 1.0f;

    if (client->getScene()) {
        logger.info("Client has a scene, creating Game instance");

        game = std::make_unique<Game>(config, *renderer, *skyboxGenerator, *planetGenerator, *registry, font, *client);
    } else {
        NEXT(checkForClientScene());
    }
}

void Application::startClient() {
    logger.info("Starting client");

    status.message = "Connecting...";
    status.value = 0.9f;

    client = std::make_unique<Client>(config, *registry, playerLocalProfile);

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

        try {
            server = std::make_unique<Server>(config, *serverCerts, *registry, *db);

            while (!server->isLoaded() && !shouldStop.load()) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }

        } catch (...) {
            EXCEPTION_NESTED("Failed to start the server");
        }

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

            db = std::make_unique<RocksDB>(path);
        } catch (...) {
            EXCEPTION_NESTED("Failed to start the database");
        }

        return [this]() { startServer(); };
    });
}

void Application::createEmptyThumbnail(Renderer& thumbnailRenderer) {
    logger.info("Creating empty thumbnail");

    thumbnailRenderer.render(nullptr, VoxelShape::Cube);
    const auto alloc = registry->getImageAtlas().add(thumbnailRenderer.getViewport(), thumbnailRenderer.getTexture());
    registry->addImage("block_empty_image", alloc);
}

void Application::createBlockThumbnails(Renderer& thumbnailRenderer) {
    logger.info("Creating block thumbnails");

    for (const auto& block : registry->getBlocks().findAll()) {
        for (const auto shape : block->getShapes()) {
            thumbnailRenderer.render(block, shape);
            const auto alloc =
                registry->getImageAtlas().add(thumbnailRenderer.getViewport(), thumbnailRenderer.getTexture());

            const auto name = fmt::format("{}_{}_image", block->getName(), VoxelShape::typeNames[shape]);
            block->setThumbnail(shape, registry->addImage(name, alloc));
        }
    }
}

void Application::createThumbnails() {
    logger.info("Creating thumbnails");

    const auto viewport = Vector2i{config.thumbnailSize, config.thumbnailSize};
    auto thumbnailRenderer =
        std::make_unique<Renderer>(config, viewport, *this, canvas, nuklear, *voxelShapeCache, *registry, font);

    createBlockThumbnails(*thumbnailRenderer);
    createEmptyThumbnail(*thumbnailRenderer);

    registry->finalize();

    if (editorOnly) {
        NEXT(createEditor());
    } else {
        NEXT(startDatabase());
    }
}

void Application::loadNextAssetInQueue(Registry::LoadQueue::const_iterator next) {
    if (next == registry->getLoadQueue().cend()) {
        NEXT(createRenderer());
    } else {
        const auto start = std::chrono::steady_clock::now();

        while (next != registry->getLoadQueue().cend()) {
            const auto count = std::distance(registry->getLoadQueue().cbegin(), next) + 1;
            const auto progress = static_cast<float>(count) / static_cast<float>(registry->getLoadQueue().size());

            try {
                (*next)(*this);
                ++next;
            } catch (...) {
                EXCEPTION_NESTED("Failed to load asset");
            }

            status.message = fmt::format("Loading assets ({}/{})...", count, registry->getLoadQueue().size());
            status.value = 0.4f + progress * 0.3f;

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

    this->registry->init(*this);

    status.message = "Loading assets...";
    status.value = 0.4f;

    loadNextAssetInQueue(registry->getLoadQueue().cbegin());
}

void Application::createRegistry() {
    logger.info("Setting up registry");

    status.message = "Loading mod packs...";
    status.value = 0.4f;

    future = std::async([this]() -> std::function<void()> {
        this->registry = std::make_unique<Registry>(config);
        return [this]() { loadAssets(); };
    });
}

void Application::createRenderer() {
    logger.info("Creating renderer");

    status.message = "Creating renderer...";
    status.value = 0.3f;

    const auto viewport = Vector2i{config.graphics.windowWidth, config.graphics.windowHeight};
    renderer = std::make_unique<Renderer>(config, viewport, *this, canvas, nuklear, *voxelShapeCache, *registry, font);

    skyboxGenerator = std::make_unique<SkyboxGenerator>(config, *this, *registry);
    planetGenerator = std::make_unique<PlanetGenerator>(config, *this, *registry);

    NEXT(createThumbnails());
}

void Application::createVoxelShapeCache() {
    logger.info("Creating voxel shape cache");

    status.message = "Creating voxel shape cache...";
    status.value = 0.4f;

    voxelShapeCache = std::make_unique<VoxelShapeCache>(config);

    NEXT(createRegistry());
}

void Application::compressAssets() {
    logger.info("Compressing assets");

    status.message = "Compressing assets (may take several minutes)...";
    status.value = 0.3f;

    future = std::async([this]() -> std::function<void()> {
        Registry::compressAssets(config);
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
        registry = std::make_unique<Registry>(config);
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
