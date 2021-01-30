#include "Application.hpp"

#include "../Assets/IconAtlas.hpp"
#include "../Assets/PbrTexture.hpp"
#include "Client.hpp"
#include "Server.hpp"

using namespace Scissio;

inline constexpr uint64_t operator|(const Application::State a, const Application::State b) {
    return static_cast<uint64_t>(a) | static_cast<uint64_t>(b);
}

inline constexpr bool operator&(const Application::State a, const uint64_t b) {
    return static_cast<uint64_t>(a) & b;
}

Application::Application(const Config& config) : OpenGLWindow("Scissio", Vector2i{1920, 1080}), config(config) {

    defaultFont = canvas.loadFont(config.assetsPath / Path("fonts") / Path("iosevka-aile-regular.ttf"));
}

Application::~Application() = default;

void Application::render(const Vector2i& viewport) {
    switch (state) {
    case State::Init: {
        try {
            eventBus = std::make_unique<EventBus>();
            textureCompressor = std::make_unique<TextureCompressor>();
            renderer = std::make_unique<Renderer>(config);
            skyboxRenderer = std::make_unique<SkyboxRenderer>(config);
            assetManager =
                std::make_unique<AssetManager>(config, canvas, *textureCompressor, *renderer, *skyboxRenderer);
            gBuffer = std::make_unique<GBuffer>();
            modManager = std::make_unique<ModManager>(*assetManager);
            modManager->load(config.assetsPath);
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
            auto server = std::make_unique<Server>(config, *assetManager);
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
            auto client = std::make_unique<Client>(config, *eventBus, *assetManager, *renderer, *skyboxRenderer, canvas,
                                                   "localhost");
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
        /*vao.bind();
        assetManager.find<IconAtlas>("icons")->getTexture().bind();
        shader.use();
        shader.setInt("tex", 0);
        shader.drawArrays(GL_TRIANGLES, 2 * 3);*/

        // const auto& icon = assetManager.find<Icon>("icons-card-10-hearts")->getImage();
        /*const auto pbr = assetManager.find<PbrTexture>("SciFiHelmet_MetallicRoughness")->getImage();

        canvas.beginFrame(viewport);
        canvas.fillColor(Color4{1.0f, 0.0f, 0.0f, 1.0f});
        canvas.beginPath();
        canvas.rectImage({0.0f, 0.0f}, {512.0f, 512.0f}, pbr);
        canvas.fill();
        canvas.closePath();
        canvas.endFrame();*/

        if (client) {
            eventBus->poll();
            client->render(*gBuffer, viewport);
        }
    }
}

void Application::eventMouseMoved(const Vector2i& pos) {
    if (client) {
        client->eventMouseMoved(pos);
    }
}

void Application::eventMousePressed(const Vector2i& pos, MouseButton button) {
    if (client) {
        client->eventMousePressed(pos, button);
    }
}

void Application::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    if (client) {
        client->eventMouseReleased(pos, button);
    }
}

void Application::eventKeyPressed(Key key) {
    if (client) {
        client->eventKeyPressed(key);
    }
}

void Application::eventKeyReleased(Key key) {
    if (client) {
        client->eventKeyReleased(key);
    }
}
