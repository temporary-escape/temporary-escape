#include "Application.hpp"

#include "../Utils/Random.hpp"
#include "Client.hpp"
#include "Server.hpp"

#include <fstream>

using namespace Scissio;

inline constexpr uint64_t operator|(const Application::State a, const Application::State b) {
    return static_cast<uint64_t>(a) | static_cast<uint64_t>(b);
}

inline constexpr bool operator&(const Application::State a, const uint64_t b) {
    return static_cast<uint64_t>(a) & b;
}

Application::Application(const Config& config) : OpenGLWindow("Scissio", Vector2i{1920, 1080}), config(config), uid{0} {

    defaultFont = canvas.loadFont(config.assetsPath / Path("fonts") / Path("iosevka-aile-regular.ttf"));
}

Application::~Application() = default;

void Application::render(const Vector2i& viewport) {
    switch (state) {
    case State::Init: {
        try {
            // eventBus = std::make_unique<EventBus>();
            textureCompressor = std::make_unique<TextureCompressor>();
            renderer = std::make_unique<Renderer>(config);
            skyboxRenderer = std::make_unique<SkyboxRenderer>(config);
            thumbnailRenderer = std::make_unique<ThumbnailRenderer>(config, *skyboxRenderer);
            assetManager = std::make_unique<AssetManager>(config, canvas, *textureCompressor, *renderer);
            gBuffer = std::make_unique<GBuffer>();
            fBuffer = std::make_unique<FBuffer>();
            modManager = std::make_unique<ModManager>(*assetManager);
            modManager->load(config.assetsPath);
            uid = generatePlayerUid();
            state = State::LoadAssets;
        } catch (...) {
            EXCEPTION_NESTED("Failed to initialize game assets");
        }
        break;
    }
    case State::LoadAssets: {
        try {
            auto& assetManagerQueue = assetManager->getLoadQueue();
            if (assetManagerQueue.empty()) {
                state = State::LoadServer;
            } else {
                const auto start = std::chrono::steady_clock::now();
                while (!assetManagerQueue.empty()) {
                    assetManagerQueue.front()();
                    assetManagerQueue.pop();

                    const auto now = std::chrono::steady_clock::now();
                    const auto test = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
                    if (test > (1.0f / 60.0f) * 1000.0f) {
                        break;
                    }
                }
            }
            break;
        } catch (...) {
            EXCEPTION_NESTED("Failed to load game assets");
        }
    }
    case State::LoadServer: {
        serverFuture = std::async([this]() {
            if (!Fs::exists(config.userdataSavesPath)) {
                Fs::create_directories(config.userdataSavesPath);
            }

            const auto savePath = config.userdataSavesPath / Path("Universe");
            if (!Fs::exists(savePath)) {
                Fs::create_directory(savePath);
            }

            const auto dbPath = savePath / Path("index.db");
            auto database = std::make_unique<Database>(dbPath);
            createSchemas(*database);
            modManager->loadXmlData(config.assetsPath, *database);

            this->database = std::move(database);

            auto server = std::make_unique<Server>(config, *assetManager, *this->database);
            server->load();
            this->server = std::move(server);
        });
        state = State::WaitServer;
        break;
    }
    case State::WaitServer: {
        if (serverFuture.ready()) {
            try {
                serverFuture.get();
                state = State::LoadClient;
            } catch (...) {
                EXCEPTION_NESTED("Failed to load server");
            }
        }
        break;
    }
    case State::LoadClient: {
        clientFuture = std::async([this]() {
            auto client = std::make_unique<Client>(config, *assetManager, *renderer, *skyboxRenderer,
                                                   *thumbnailRenderer, canvas, "localhost", uid);
            this->client = std::move(client);
        });
        state = State::WaitClient;
        break;
    }
    case State::WaitClient: {
        if (clientFuture.ready()) {
            try {
                clientFuture.get();
                state = State::Playing;
            } catch (...) {
                EXCEPTION_NESTED("Failed to load client");
            }
        }
        break;
    }
    default: {
        break;
    }
    }

    if (viewport == Vector2i{0, 0}) {
        return;
    }

    glViewport(0, 0, viewport.x, viewport.y);
    Framebuffer::DefaultFramebuffer.bind();
    static Color4 black{0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, &black.x);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
    // glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static constexpr auto LOADING_STATE = static_cast<uint64_t>(State::LoadAssets) | static_cast<uint64_t>(State::Init);
    if (static_cast<uint64_t>(state) & LOADING_STATE) {
        const auto progress = std::log2f(static_cast<float>(state)) / 8.0f;

        canvas.beginFrame(viewport);
        canvas.fontFace(defaultFont);
        canvas.fontSize(24.0f);
        canvas.beginPath();
        canvas.fillColor({1.0f, 1.0f, 1.0f, 1.0f});
        canvas.rect({50.0f, viewport.y - 75.0f}, {(viewport.x - 100.0f) * progress, 25.0f});
        canvas.fill();
        canvas.text({50.0f, viewport.y - 100.0f}, "Loading...");
        canvas.closePath();
        canvas.endFrame();
    }

    else if (state == State::Playing) {
        if (client) {
            client->update();
            client->render(*gBuffer, *fBuffer, viewport);
        }
    }
}

void Application::eventMouseMoved(const Vector2i& pos) {
    if (client) {
        client->eventMouseMoved(pos);
    }
}

void Application::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (client) {
        client->eventMousePressed(pos, button);
    }
}

void Application::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (client) {
        client->eventMouseReleased(pos, button);
    }
}

void Application::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (client) {
        client->eventKeyPressed(key, modifiers);
    }
}

void Application::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (client) {
        client->eventKeyReleased(key, modifiers);
    }
}

void Application::eventMouseScroll(const int xscroll, const int yscroll) {
    if (client) {
        client->eventMouseScroll(xscroll, yscroll);
    }
}

uint64_t Application::generatePlayerUid() {
    const auto path = config.userdataPath / Path("uid");
    if (Fs::exists(path)) {
        std::ifstream f(path);
        if (!f.is_open()) {
            EXCEPTION("Failed to open '{}' for reading", path.string());
        }

        const std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        return std::stoull(str);
    } else {
        const auto uid = randomId();

        std::ofstream f(path);
        if (!f.is_open()) {
            EXCEPTION("Failed to open '{}' for writing", path.string());
        }

        f << uid;

        return uid;
    }
}
