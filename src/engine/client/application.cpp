#include "application.hpp"
#include "../graphics/theme.hpp"

#define CMP "Application"

using namespace Engine;

Application::Application(const Config& config) :
    VulkanRenderer{config},
    config{config},
    renderer{config, *this},
    skyboxGenerator{config, *this},
    canvas{*this},
    font{*this, config.fontsPath, "iosevka-aile", 42.0f},
    nuklear{canvas, font, config.guiFontSize} {

    gui.mainMenu.setItems({
        {"Singleplayer", [this]() { startSinglePlayer(); }},
        {"Multiplayer", []() {}},
        {"Settings", []() {}},
        {"Mods", []() {}},
        {"Exit", [this]() { closeWindow(); }},
    });
    gui.mainMenu.setFontSize(config.guiFontSize * 1.25f);

    status.value = 1.0f;
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
    if (status.value < 1.0f) {
        renderStatus(viewport);
    }
    nuklear.begin(viewport);

    nuklear.draw(gui.mainMenu);

    nuklear.end();
    canvas.end(vkb);

    /*cmd.bindPipeline(pipeline);
    cmd.setViewport({0, 0}, viewport);
    cmd.setScissor({0, 0}, viewport);
    cmd.bindBuffers({{vbo, 0}});
    cmd.bindIndexBuffer(ibo, 0, VkIndexType::VK_INDEX_TYPE_UINT16);
    cmd.bindDescriptors(pipeline, descriptorSetLayout, {{0, &ubo.getCurrentBuffer()}}, {{1, &texture}});
    cmd.drawIndexed(indices.size(), 1, 0, 0, 0);*/

    vkb.endRenderPass();
    vkb.end();

    submitCommandBuffer(vkb);

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

void Application::startClient() {
    status.message = "Connecting...";
    status.value = 0.9f;
}

void Application::loadServer() {
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
        NEXT(startDatabase());
    } else {
        const auto count = std::distance(registry->getLoadQueue().cbegin(), next);
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

void Application::createRegistry() {
    status.message = "Loading mod packs...";
    status.value = 0.3f;

    future = std::async([this]() -> std::function<void()> {
        this->registry = std::make_unique<Registry>(config);
        return [this]() {
            this->registry->init(*this);

            status.message = "Loading assets...";
            status.value = 0.4f;

            loadNextAssetInQueue(registry->getLoadQueue().cbegin());
        };
    });
}

void Application::compileShaders() {
    status.message = "Loading shaders...";
    status.value = 0.1f;

    shaderLoadQueue = shaders.createLoadQueue();
    NEXT(compileNextShaderInQueue(shaderLoadQueue.begin()));
}

void Application::compileNextShaderInQueue(Renderer::ShaderLoadQueue::iterator next) {
    if (next == shaderLoadQueue.end()) {
        NEXT(createRegistry());
    }
    // Not yet done, we have more shaders to compile
    else {
        const auto count = std::distance(shaderLoadQueue.begin(), next);
        const auto progress = static_cast<float>(count) / static_cast<float>(shaderLoadQueue.size());

        try {
            (*next)(config, *this);
            ++next;
        } catch (...) {
            EXCEPTION_NESTED("Failed to compile shader");
        }

        status.message = fmt::format("Loading shaders ({}/{})...", count, shaderLoadQueue.size());
        status.value = 0.1f + progress * 0.2f;

        NEXT(compileNextShaderInQueue(next));
    }
}

void Application::startSinglePlayer() {
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
    nuklear.eventMouseMoved(pos);
    if (game) {
        game->eventMouseMoved(pos);
    }
}

void Application::eventMousePressed(const Vector2i& pos, MouseButton button) {
    nuklear.eventMousePressed(pos, button);
    if (game) {
        game->eventMousePressed(pos, button);
    }
}

void Application::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    nuklear.eventMouseReleased(pos, button);
    if (game) {
        game->eventMouseReleased(pos, button);
    }
}

void Application::eventMouseScroll(const int xscroll, const int yscroll) {
    nuklear.eventMouseScroll(xscroll, yscroll);
    if (game) {
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
