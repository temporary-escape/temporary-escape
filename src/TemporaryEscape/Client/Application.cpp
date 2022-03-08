#include "Application.hpp"
#include "../Graphics/Framebuffer.hpp"
#include <functional>
#include <thread>

#define CMP "Application"

using namespace Engine;

Application::Application(Config& config, const Options& options)
    : OpenGLWindow("Engine Game", {config.windowWidth, config.windowHeight}), config(config), options(options),
      loading(true), loadingProgress(0.0f) {
    defaultFont = canvas.loadFont(config.assetsPath / "base" / "fonts" / Path(config.guiFontFaceRegular + ".ttf"));

    audioContext = std::make_shared<AudioContext>();

    textureCompressor = std::make_shared<TextureCompressor>();
}

Application::~Application() {
}

void Application::update() {
    if (!stagesFuture.valid() && !assetManager) {
        loadingProgress.store(0.1f);
        loadQueueFuture = loadQueuePromise.future();
        stagesFuture = std::async(std::bind(&Application::load, this));
    }

    try {
        if (stagesFuture.valid()) {
            if (loadQueueFuture.valid() && loadQueueFuture.ready()) {
                assetLoadQueue = loadQueueFuture.get();
            }

            auto wasNotEmpty = !assetLoadQueue.empty();

            const auto start = std::chrono::steady_clock::now();
            while (!assetLoadQueue.empty()) {
                assetLoadQueue.front()();
                assetLoadQueue.pop();

                const auto now = std::chrono::steady_clock::now();
                const auto test = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
                if (test > (1.0f / 60.0f) * 1000.0f) {
                    break;
                }

                loadingProgress.store((assetLoadQueue.size() / assetLoadQueueInitialSize.load()) * 0.7f + 0.1f);
            }

            if (wasNotEmpty && assetLoadQueue.empty()) {
                assetLoadQueueFinished.resolve(true);
            }
        }
    } catch (...) {
        assetLoadQueueFinished.resolve(false);
        EXCEPTION_NESTED("Failed to load assets");
    }

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
    } else if (client) {
        if (!renderer) {
            renderer = std::make_shared<Renderer>(config, canvas, *assetManager);
            view = std::make_shared<ViewRoot>(config, canvas, *assetManager, *renderer, *client);
        }

        view->render(viewport);
    }
}

void Application::load() {
    Log::i(CMP, "Initializing resource managers");
    assetManager = std::make_shared<AssetManager>(config, canvas, *textureCompressor);
    modManager = std::make_shared<ModManager>();

    Log::i(CMP, "Loading mods");
    modManager->load(*assetManager, config.assetsPath / "base");
    auto queue = assetManager->getLoadQueue();
    assetLoadQueueInitialSize.store(queue.size());
    loadQueuePromise.resolve(std::move(queue));

    Log::i(CMP, "Waiting for assets to load");
    try {
        auto future = assetLoadQueueFinished.future();
        if (!future.get()) {
            return;
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to wait for assets to load");
    }

    loadingProgress.store(0.8f);

    auto saveDir = config.userdataSavesPath;

    if (options.saveFolderName.has_value()) {
        saveDir /= options.saveFolderName.value();
    } else {
        saveDir /= "Universe";
    }

    Log::i(CMP, "Using save directory: {}", saveDir.string());

    if (options.saveFolderClean) {
        Log::w(CMP, "Deleting save directory");
        Fs::remove_all(saveDir);
    }

    if (!Fs::exists(saveDir) && !Fs::create_directories(saveDir)) {
        EXCEPTION("Failed to create save directory: '{}'", saveDir.string());
    }

    db = std::make_shared<Database>(saveDir / "database");

    loadingProgress.store(0.8f);

    Log::i(CMP, "Starting server");
    server = std::make_shared<Server>(config, *assetManager, *db);
    server->load();

    loadingProgress.store(0.9f);

    Log::i(CMP, "Starting client");
    client = std::make_shared<Client>(config, "localhost", config.serverPort);

    loadingProgress.store(1.0f);

    loading.store(false);
    Log::i(CMP, "Loading finished");
}

void Application::eventMouseMoved(const Vector2i& pos) {
    if (view) {
        view->eventMouseMoved(pos);
    }
}

void Application::eventMousePressed(const Vector2i& pos, MouseButton button) {
    if (view) {
        view->eventMousePressed(pos, button);
    }
}

void Application::eventMouseReleased(const Vector2i& pos, MouseButton button) {
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
    if (key == Key::LetterR) {
        renderer->reloadShaders();
    }
    if (view) {
        view->eventKeyPressed(key, modifiers);
    }
}

void Application::eventKeyReleased(Key key, Modifiers modifiers) {
    if (view) {
        view->eventKeyReleased(key, modifiers);
    }
}
