#include "Application.hpp"
#include "../Graphics/Framebuffer.hpp"
#include <functional>
#include <thread>

#define CMP "Application"

using namespace Scissio;

Application::Application(Config& config) : OpenGLWindow("Scissio Game", {config.windowWidth, config.windowHeight}), config(config), loading(true), loadingProgress(0.0f) {
    defaultFont = canvas.loadFont(config.assetsPath / Path("fonts") / Path(config.guiFontFaceRegular));

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

    if (loading.load()) {
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
    }
}

void Application::load() {
    Log::i(CMP, "Initializing resource managers");
    assetManager = std::make_shared<AssetManager>(config, *textureCompressor);
    modManager = std::make_shared<ModManager>();

    Log::i(CMP, "Loading mods");
    modManager->load(*assetManager, config.assetsPath);
    auto queue = assetManager->getLoadQueue();
    assetLoadQueueInitialSize.store(queue.size());
    loadQueuePromise.resolve(std::move(queue));

    auto future = assetLoadQueueFinished.future();
    if (!future.get()) {
        return;
    }

    loadingProgress.store(0.8f);

    const auto saveDir = config.userdataSavesPath / "Universe";
    if (!std::filesystem::exists(saveDir) && !std::filesystem::create_directories(saveDir)) {
        EXCEPTION("Failed to create save directory: '{}'", saveDir.string());
    }

    db = std::make_shared<Database>(saveDir / "database");

    loadingProgress.store(0.8f);

    generator = std::make_shared<Generator>(*db, Generator::Options{});
    generator->generateWorld(123456789u);

    loadingProgress.store(0.9f);

    server = std::make_shared<Server>(config, *assetManager, *db);

    loadingProgress.store(0.95f);

    client = std::make_shared<Client>(config, "localhost", config.serverPort);
    client->login("password");

    loadingProgress.store(1.0f);

    loading.store(false);
}

void Application::eventMouseMoved(const Vector2i& pos) {
}

void Application::eventMousePressed(const Vector2i& pos, MouseButton button) {
}

void Application::eventMouseReleased(const Vector2i& pos, MouseButton button) {
}

void Application::eventMouseScroll(int xscroll, int yscroll) {
}

void Application::eventKeyPressed(Key key, Modifiers modifiers) {
}

void Application::eventKeyReleased(Key key, Modifiers modifiers) {
}
