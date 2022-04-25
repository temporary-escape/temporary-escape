#include "Application.hpp"
#include "../Graphics/Framebuffer.hpp"
#include <functional>
#include <thread>

#define CMP "Application"

using namespace Engine;

Application::Application(Config& config)
    : OpenGLWindow("Engine Game", {config.windowWidth, config.windowHeight}), config(config), loading(true),
      loadingProgress(0.0f) {
    defaultFont = canvas.loadFont(config.assetsPath / "base" / "fonts" / Path(config.guiFontFaceRegular + ".ttf"));

    audioContext = std::make_shared<AudioContext>();

    textureCompressor = std::make_shared<TextureCompressor>();
    modManager = std::make_shared<ModManager>();
    shaders = std::make_shared<Shaders>(config);
    gbuffer = std::make_shared<GBuffer>();
    skyboxRenderer = std::make_shared<SkyboxRenderer>(config);
    stats = std::make_shared<Stats>();
}

Application::~Application() {
}

void Application::update() {
    if (!stagesFuture.valid() && !assetManager) {
        loadingProgress.store(0.1f);
        // loadQueueFuture = loadQueuePromise.future();
        assetManager = std::make_shared<AssetManager>(config, canvas, *textureCompressor);
        gridBuilder = std::make_shared<Grid::Builder>(*assetManager);
        renderer = std::make_shared<Renderer>(config, *shaders, *skyboxRenderer, *gridBuilder);
        imager = std::make_shared<Imager>(config, *assetManager, *renderer);
        stagesFuture = std::async(std::bind(&Application::load, this));
    }

    worker.restart();
    worker.run_for(std::chrono::milliseconds(10));

    try {
        if (stagesFuture.valid() && stagesFuture.ready()) {
            stagesFuture.get();
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to initialize game resources");
    }
}

void Application::render(const Vector2i& viewport) {
    update();

    gBuffer.resize(viewport);
    renderer->setViewport(viewport);
    renderer->setGBuffer(gBuffer);

    glViewport(0, 0, viewport.x, viewport.y);
    Framebuffer::DefaultFramebuffer.bind();
    static Color4 black{0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, &black.x);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    if (client) {
        client->update();
    }

    if (loading.load() || (client && client->getScene() == nullptr)) {
        canvas.beginFrame(viewport);
        canvas.fontFace(defaultFont);
        canvas.fontSize(config.guiFontSize);
        canvas.beginPath();
        canvas.fillColor({1.0f, 1.0f, 1.0f, 1.0f});
        canvas.rect({50.0f, viewport.y - 75.0f}, {(viewport.x - 100.0f) * loadingProgress.load(), 25.0f});
        canvas.fill();
        canvas.text({50.0f, viewport.y - 100.0f}, "Loading...");
        canvas.closePath();
        canvas.endFrame();
    } else if (client && !viewSpace) {
        gui = std::make_shared<GuiContext>(canvas, config, *assetManager);
        widgets = std::make_shared<Widgets>(*gui);
        viewSpace = std::make_shared<ViewSpace>(config, canvas, *assetManager, *renderer, *client, *widgets);
        viewMap = std::make_shared<ViewMap>(config, canvas, *assetManager, *renderer, *client, *widgets);
        viewBuild = std::make_shared<ViewBuild>(config, canvas, *assetManager, *renderer, *widgets);
        view = viewSpace.get();
    } else if (config.voxelTest && !viewVoxelTest) {
        gui = std::make_shared<GuiContext>(canvas, config, *assetManager);
        widgets = std::make_shared<Widgets>(*gui);
        viewVoxelTest = std::make_shared<ViewVoxelTest>(config, canvas, *assetManager, *renderer, *widgets);
        view = viewVoxelTest.get();
    }

    if (view) {
        try {
            renderView(viewport);
        } catch (...) {
            EXCEPTION_NESTED("Failed to render the view");
        }
    }
}

void Application::renderView(const Vector2i& viewport) {
    const auto t0 = std::chrono::steady_clock::now();

    renderer->setEnableBackground(true);

    try {
        view->render(viewport);
    } catch (...) {
        EXCEPTION_NESTED("Failed to render the scene");
    }

    renderer->blit(viewport, Framebuffer::DefaultFramebuffer);
    Framebuffer::DefaultFramebuffer.bind();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    canvas.beginFrame(viewport);
    gui->reset();

    widgets->update(viewport);

    try {
        view->renderGui(viewport);

        gui->render(viewport);

    } catch (...) {
        EXCEPTION_NESTED("Failed to render the gui");
    }

    canvas.endFrame();

    const auto t1 = std::chrono::steady_clock::now();
    const auto tDiff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    // client.getStats().render.frameTimeMs.store(tDiff.count());
}

void Application::load() {
    Log::i(CMP, "Loading mods");
    modManager->load(*assetManager, config.assetsPath / "base");
    auto queue = assetManager->getLoadQueue();

    std::atomic<bool> error(false);
    std::shared_ptr<Promise<void>> promise;

    auto const wait = [&]() {
        promise = std::make_shared<Promise<void>>();
        worker.post([promise]() { promise->resolve(); });
        promise->future().get();

        if (error.load()) {
            loading.store(false);
            EXCEPTION("Failed to load assets, see the log for errors");
        }
    };

    Log::i(CMP, "Waiting for assets to load...");
    while (!queue.empty()) {
        AssetLoadQueue::value_type item;
        std::swap(item, queue.front());
        queue.pop();

        worker.post([promise, &error, item = std::move(item)]() {
            try {
                item();
            } catch (std::exception& e) {
                BACKTRACE(CMP, e, "Failed to load asset");
                error.store(true);
            }
        });
    }
    wait();

    Log::i(CMP, "Waiting for grid cache to build...");
    worker.post([promise, &error, this]() {
        try {
            gridBuilder->precompute();
        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Failed to build grid cache");
            error.store(true);
        }
    });
    wait();

    Log::i(CMP, "Waiting for thumbnails to render...");
    for (const auto& block : assetManager->findAll<AssetBlock>()) {
        worker.post([promise, block, this]() {
            try {
                imager->createThumbnail(block);
            } catch (std::exception& e) {
                BACKTRACE(CMP, e, "Failed to render thumbnail");
            }
        });
    }
    wait();

    if (config.voxelTest) {
        loadingProgress.store(1.0f);
        loading.store(false);
        Log::i(CMP, "Loading finished");
        return;
    }

    loadingProgress.store(0.8f);

    auto saveDir = config.userdataSavesPath;

    if (config.saveFolderName.has_value()) {
        saveDir /= config.saveFolderName.value();
    } else {
        saveDir /= "Universe";
    }

    Log::i(CMP, "Using save directory: {}", saveDir.string());

    if (config.saveFolderClean) {
        Log::w(CMP, "Deleting save directory");
        Fs::remove_all(saveDir);
    }

    if (!Fs::exists(saveDir) && !Fs::create_directories(saveDir)) {
        EXCEPTION("Failed to create save directory: '{}'", saveDir.string());
    }

    db = std::make_shared<RocksDB>(saveDir / "database");

    loadingProgress.store(0.8f);

    Log::i(CMP, "Starting server");
    server = std::make_shared<Server>(config, *assetManager, *db);
    server->load();

    loadingProgress.store(0.9f);

    Log::i(CMP, "Starting client");
    client = std::make_shared<Client>(config, *stats, "localhost", config.serverPort);

    loadingProgress.store(1.0f);

    loading.store(false);
    Log::i(CMP, "Loading finished");
}

void Application::eventMouseMoved(const Vector2i& pos) {
    if (gui) {
        gui->mouseMoveEvent(pos);
    }
    if (view) {
        view->eventMouseMoved(pos);
    }
}

void Application::eventMousePressed(const Vector2i& pos, MouseButton button) {
    if (gui) {
        gui->mousePressEvent(pos, button);
    }
    if (view) {
        view->eventMousePressed(pos, button);
    }
}

void Application::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    if (gui) {
        gui->mouseReleaseEvent(pos, button);
    }
    if (view) {
        view->eventMouseReleased(pos, button);
    }
}

void Application::eventMouseScroll(int xscroll, int yscroll) {
    if (view) {
        view->eventMouseScroll(xscroll, yscroll);
    }
}

void Application::eventKeyPressed(Key key, Modifiers modifiers) {
    if (gui) {
        gui->keyPressEvent(key);
    }
    if (key == Key::LetterR) {
        try {
            Log::i(CMP, "Reloading shaders...");
            *shaders = Shaders(config);
        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Failed to reload shaders");
        }
    }

    if (key == Key::LetterM && viewMap && viewSpace) {
        if (view != viewMap.get()) {
            viewMap->load();
            view = viewMap.get();
        } else {
            view = viewSpace.get();
        }
    }

    if (key == Key::LetterB && viewBuild && viewSpace) {
        if (view != viewBuild.get()) {
            viewBuild->load();
            view = viewBuild.get();
        } else {
            view = viewSpace.get();
        }
    }

    if (view) {
        view->eventKeyPressed(key, modifiers);
    }
}

void Application::eventKeyReleased(Key key, Modifiers modifiers) {
    if (gui) {
        gui->keyReleaseEvent(key);
    }
    if (view) {
        view->eventKeyReleased(key, modifiers);
    }
}
