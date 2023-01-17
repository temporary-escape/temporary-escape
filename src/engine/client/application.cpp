#include "application.hpp"
#include "../graphics/theme.hpp"

#define CMP "Application"

using namespace Engine;

Application::Application(const Config& config) :
    VulkanRenderer{config},
    config{config},
    canvas{*this},
    font{*this, config.fontsPath, "iosevka-aile", config.guiFontSize * 2.0f},
    nuklear{canvas, font, config.guiFontSize} {

    gui.mainMenu.setItems({
        {"Singleplayer", [this]() { startSinglePlayer(); }},
        {"Multiplayer", []() {}},
        {"Settings", []() {}},
        {"Mods", []() {}},
        {"Exit", [this]() { closeWindow(); }},
    });
    gui.mainMenu.setFontSize(config.guiFontSize * 1.25f);

    startSinglePlayer();
}

Application::~Application() {
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

    if (game) {
        game->update(deltaTime);
        game->render(viewport);
        return;
    }

    auto vkb = createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &getSwapChainFramebuffer();
    renderPassInfo.renderPass = &getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValues = {clearColor};

    vkb.beginRenderPass(renderPassInfo);

    canvas.begin(viewport);
    if (!status.message.empty()) {
        renderStatus(viewport);
    }
    nuklear.begin(viewport);

    nuklear.draw(gui.mainMenu);

    nuklear.end();
    canvas.end(vkb);

    vkb.endRenderPass();
    vkb.end();

    submitPresentCommandBuffer(vkb);

    dispose(std::move(vkb));
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

void Application::checkForClientScene() {
    status.message = "Entering...";
    status.value = 1.0f;

    if (client->getScene()) {
        Log::i(CMP, "Client has a scene, creating Game instance");

        game = std::make_unique<Game>(config, *renderer, canvas, nuklear, *skyboxGenerator, *registry, font, *client);
    } else {
        NEXT(createRegistry());
    }
}

void Application::startClient() {
    Log::i(CMP, "Starting client");

    status.message = "Connecting...";
    status.value = 0.9f;

    client = std::make_unique<Client>(config, *registry, playerLocalProfile);

    Log::i(CMP, "Connecting to the server");

    future = std::async([this]() -> std::function<void()> {
        client->connect("localhost", config.serverPort);

        return [this]() { checkForClientScene(); };
    });
}

void Application::loadServer() {
    Log::i(CMP, "Loading server");

    status.message = "Loading universe...";
    status.value = 0.8f;

    future = std::async([this]() -> std::function<void()> {
        serverCerts = std::make_unique<Server::Certs>();

        try {
            server->load();
        } catch (...) {
            EXCEPTION_NESTED("Failed to load the server");
        }

        return [this]() { startClient(); };
    });
}

void Application::startServer() {
    Log::i(CMP, "Starting server");

    status.message = "Starting server...";
    status.value = 0.75f;

    future = std::async([this]() -> std::function<void()> {
        serverCerts = std::make_unique<Server::Certs>();

        try {
            server = std::make_unique<Server>(config, *serverCerts, *registry, *db);
        } catch (...) {
            EXCEPTION_NESTED("Failed to start the server");
        }

        return [this]() { loadServer(); };
    });
}

void Application::startDatabase() {
    Log::i(CMP, "Starting database");

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
                Log::w(CMP, "Deleting save: '{}'", path);
                Fs::remove_all(path);
            }
            if (!Fs::exists(path)) {
                Log::i(CMP, "Creating save: '{}'", path);
                Fs::create_directories(path);
            }
            Log::i(CMP, "Starting database with save: '{}'", path);

            db = std::make_unique<RocksDB>(path);
        } catch (...) {
            EXCEPTION_NESTED("Failed to start the database");
        }

        return [this]() { startServer(); };
    });
}

void Application::loadNextAssetInQueue(Registry::LoadQueue::const_iterator next) {
    if (next == registry->getLoadQueue().cend()) {
        registry->finalize();
        NEXT(startDatabase());
    } else {
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

        NEXT(loadNextAssetInQueue(next));
    }
}

void Application::loadAssets() {
    Log::i(CMP, "Loading assets");

    this->registry->init(*this);

    status.message = "Loading assets...";
    status.value = 0.4f;

    loadNextAssetInQueue(registry->getLoadQueue().cbegin());
}

void Application::createRegistry() {
    Log::i(CMP, "Setting up registry");

    status.message = "Loading mod packs...";
    status.value = 0.3f;

    future = std::async([this]() -> std::function<void()> {
        this->registry = std::make_unique<Registry>(config);
        return [this]() { loadAssets(); };
    });
}

void Application::createRenderer() {
    Log::i(CMP, "Creating renderer");

    status.message = "Creating renderer...";
    status.value = 0.3f;

    renderer = std::make_unique<Renderer>(config, *this, canvas, nuklear, *shaderModules, *voxelShapeCache, font);
    skyboxGenerator = std::make_unique<SkyboxGenerator>(config, *this, *shaderModules);

    NEXT(createRegistry());
}

void Application::createVoxelShapeCache() {
    Log::i(CMP, "Creating voxel shape cache");

    status.message = "Creating voxel shape cache...";
    status.value = 0.3f;

    voxelShapeCache = std::make_unique<VoxelShapeCache>(config);

    NEXT(createRenderer());
}

void Application::compileShaders() {
    Log::i(CMP, "Compiling shaders");

    status.message = "Loading shaders...";
    status.value = 0.1f;

    shaderModules = std::make_unique<ShaderModules>(config, *this);
    NEXT(compileNextShaderInQueue(shaderModules->getLoadQueue().begin()));
}

void Application::compileNextShaderInQueue(ShaderModules::LoadQueue::iterator next) {
    if (next == shaderModules->getLoadQueue().end()) {
        NEXT(createVoxelShapeCache());
    }
    // Not yet done, we have more shaders to compile
    else {
        const auto count = std::distance(shaderModules->getLoadQueue().begin(), next) + 1;
        const auto progress = static_cast<float>(count) / static_cast<float>(shaderModules->getLoadQueue().size());

        try {
            (*next)();
            ++next;
        } catch (...) {
            EXCEPTION_NESTED("Failed to compile shader");
        }

        status.message = fmt::format("Loading shaders ({}/{})...", count, shaderModules->getLoadQueue().size());
        status.value = 0.1f + progress * 0.2f;

        NEXT(compileNextShaderInQueue(next));
    }
}

void Application::startSinglePlayer() {
    Log::i(CMP, "Starting single player mode");

    status.message = "Loading...";
    status.value = 0.0f;

    gui.mainMenu.setEnabled(false);

    NEXT(compileShaders());

    // game = std::make_unique<Game>(config, *this, canvas, font, nuklear, skyboxGenerator);
    /*future = std::async([]() {
        registry = std::make_unique<Registry>(config);
    });*/
}

void Application::eventMouseMoved(const Vector2i& pos) {
    mousePos = pos;
    nuklear.eventMouseMoved(pos);
    if (game && !nuklear.isCursorInsideWindow(pos)) {
        game->eventMouseMoved(pos);
    }
}

void Application::eventMousePressed(const Vector2i& pos, MouseButton button) {
    mousePos = pos;
    nuklear.eventMousePressed(pos, button);
    if (game && !nuklear.isCursorInsideWindow(pos)) {
        game->eventMousePressed(pos, button);
    }
}

void Application::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    mousePos = pos;
    nuklear.eventMouseReleased(pos, button);
    if (game && !nuklear.isCursorInsideWindow(pos)) {
        game->eventMouseReleased(pos, button);
    }
}

void Application::eventMouseScroll(const int xscroll, const int yscroll) {
    nuklear.eventMouseScroll(xscroll, yscroll);
    if (game && !nuklear.isCursorInsideWindow(mousePos)) {
        game->eventMouseScroll(xscroll, yscroll);
    }
}

void Application::eventKeyPressed(const Key key, const Modifiers modifiers) {
    nuklear.eventKeyPressed(key, modifiers);
    if (game) {
        game->eventKeyPressed(key, modifiers);
    }
}

void Application::eventKeyReleased(const Key key, const Modifiers modifiers) {
    nuklear.eventKeyReleased(key, modifiers);
    if (game) {
        game->eventKeyReleased(key, modifiers);
    }
}

void Application::eventWindowResized(const Vector2i& size) {
}

void Application::eventCharTyped(const uint32_t code) {
    nuklear.eventCharTyped(code);
    if (game) {
        game->eventCharTyped(code);
    }
}

void Application::eventWindowBlur() {
}

void Application::eventWindowFocus() {
}
