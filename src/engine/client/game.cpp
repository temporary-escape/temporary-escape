#include "game.hpp"
#include "../graphics/theme.hpp"
#include "../utils/random.hpp"

#define CMP "Game"

using namespace Engine;

Game::Game(const Config& config, Renderer& renderer, Canvas& canvas, Nuklear& nuklear, SkyboxGenerator& skyboxGenerator,
           Registry& registry, Client& client) :
    config{config},
    renderer{renderer},
    canvas{canvas},
    nuklear{nuklear},
    skyboxGenerator{skyboxGenerator},
    registry{registry},
    client{client},
    skybox{renderer.getVulkan(), Color4{0.03f, 0.03f, 0.03f, 1.0f}} {

    viewSpace = std::make_unique<ViewSpace>(config, renderer, registry, skybox, client);
    viewGalaxy = std::make_unique<ViewGalaxy>(config, renderer, registry, client);
    viewSystem = std::make_unique<ViewSystem>(config, renderer, registry, client);
    view = viewSpace.get();
}

Game::~Game() = default;

void Game::update(float deltaTime) {
    /*if (!viewBuild) {
        viewBuild = std::make_unique<ViewBuild>(config, vulkan, scenePipelines, registry, canvas, font, nuklear);
        view = viewBuild.get();
    }*/

    /*if (client && viewSpace && client->getScene() != nullptr && view == nullptr) {
        view = viewSpace.get();
    }

    if (client && clientLoad.valid() && clientLoad.ready()) {
        clientLoad.get();

        Log::i(CMP, "Starting scene");
        status.message = "Entering universe...";
        status.value = 1.0f;

        viewSpace = std::make_unique<ViewSpace>(config, vulkan, registry, skyboxGenerator.get(), *client);

        viewGalaxy = std::make_unique<ViewGalaxy>(config, vulkan, registry, *client);
        viewSystem = std::make_unique<ViewSystem>(config, vulkan, registry, *client);
    }

    if (server && serverLoad.valid() && serverLoad.ready() && !client) {
        serverLoad.get();

        Log::i(CMP, "Starting client");
        status.message = "Connecting...";
        status.value = 0.9f;

        const auto profilePath = config.userdataPath / "profile.yml";
        if (!Fs::exists(profilePath)) {
            PlayerLocalProfile localProfile{};
            localProfile.secret = randomId();
            localProfile.name = "Some Player";
            localProfile.toYaml(profilePath);
        }

        client = std::make_unique<Client>(config, registry, stats, profilePath);

        // Client event: sector data retrieved
        client->onSectorUpdated([this](SystemData& system) {
            // When we change sector we must recreate the skybox!
            skyboxGenerator.updateSeed(//system.seed 987654321ULL);
        });

        clientLoad = client->connect("localhost", config.serverPort);
    }

    if (db && !server) {
        Log::i(CMP, "Starting server");
        status.message = "Starting server...";
        status.value = 0.8f;

        server = std::make_unique<Server>(config, serverCerts, registry, *db);
        serverLoad = server->load();
    }

    if (!db) {
        Log::i(CMP, "Starting database");
        status.message = "Starting database...";
        status.value = 0.8f;

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
    }

    if (client) {
        client->update();
    }

    if (view) {
        view->update(deltaTime);
    }*/
}

void Game::render(const Vector2i& viewport) {
    auto& vulkan = renderer.getVulkan();

    auto vkb = vulkan.createCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkb.start(beginInfo);

    VulkanRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.framebuffer = &vulkan.getSwapChainFramebuffer();
    renderPassInfo.renderPass = &vulkan.getRenderPass();
    renderPassInfo.offset = {0, 0};
    renderPassInfo.size = viewport;

    VkClearValue clearColor = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
    renderPassInfo.clearValues = {clearColor};

    vkb.beginRenderPass(renderPassInfo);

    /*canvas.begin(viewport);
    renderStatus(viewport);
    canvas.end(vkb);*/

    vkb.endRenderPass();
    vkb.end();

    vulkan.submitPresentCommandBuffer(vkb);
    vulkan.dispose(std::move(vkb));
}

/*void Game::renderCanvas(const Vector2i& viewport) {
    canvas.begin(viewport);

    if (view) {
        view->renderCanvas(viewport, canvas);

        nuklear.begin(viewport);
        view->renderGui(viewport, nuklear);
        nuklear.end();
    }

    canvas.end();
}*/

void Game::eventMouseMoved(const Vector2i& pos) {
    if (view) {
        view->eventMouseMoved(pos);
    }
}

void Game::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (view) {
        view->eventMousePressed(pos, button);
    }
}

void Game::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (view) {
        view->eventMouseReleased(pos, button);
    }
}

void Game::eventMouseScroll(const int xscroll, const int yscroll) {
    if (view) {
        view->eventMouseScroll(xscroll, yscroll);
    }
}

void Game::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (view) {
        view->eventKeyPressed(key, modifiers);
    }

    /*if (config.input.galaxyMapToggle(key, modifiers)) {
        if (view != viewGalaxy.get()) {
            Log::i(CMP, "Switching to galaxy map scene...");
            view = viewGalaxy.get();
            viewGalaxy->load();
        } else {
            Log::i(CMP, "Switching to space scene...");
            view = viewSpace.get();
        }
    } else if (config.input.systemMapToggle(key, modifiers)) {
        if (view != viewSystem.get()) {
            Log::i(CMP, "Switching to system map scene...");
            view = viewSystem.get();
            viewSystem->load();
        } else {
            Log::i(CMP, "Switching to system scene...");
            view = viewSpace.get();
        }
    }*/
}

void Game::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (view) {
        view->eventKeyReleased(key, modifiers);
    }
}

void Game::eventCharTyped(const uint32_t code) {
    if (view) {
        view->eventCharTyped(code);
    }
}
