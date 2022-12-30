#include "game.hpp"
#include "../utils/random.hpp"

#define CMP "Game"

using namespace Engine;

Game::Game(const Config& config, VulkanRenderer& vulkan, Registry& registry, Canvas& canvas, FontFamily& font,
           SkyboxGenerator& skyboxGenerator, Status& status) :
    config{config},
    vulkan{vulkan},
    registry{registry},
    canvas{canvas},
    font{font},
    skyboxGenerator{skyboxGenerator},
    status{status},
    nuklear{canvas, font.regular, 19.0f},
    serverCerts{} {
}

Game::~Game() {
    Log::i(CMP, "Game destruct");
    if (client) {
        client->stop();
    }
    client.reset();
    if (server) {
        server->stop();
    }
    server.reset();
}

void Game::update(float deltaTime) {
    /*if (!viewBuild) {
        viewBuild = std::make_unique<ViewBuild>(config, vulkan, scenePipelines, registry, canvas, font, nuklear);
        view = viewBuild.get();
    }*/

    if (client && viewSpace && client->getScene() != nullptr && view == nullptr) {
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
            skyboxGenerator.updateSeed(/*system.seed*/ 987654321ULL);
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
    }
}

void Game::render(const Vector2i& viewport, Renderer& renderer) {
    if (view) {
        view->render(viewport, renderer);
    }
}

void Game::renderCanvas(const Vector2i& viewport) {
    /*canvas.begin(viewport);

    if (view) {
        view->renderCanvas(viewport, canvas);

        nuklear.begin(viewport);
        view->renderGui(viewport, nuklear);
        nuklear.end();
    }

    canvas.end();*/
}

void Game::eventMouseMoved(const Vector2i& pos) {
    nuklear.eventMouseMoved(pos);
    if (view) {
        view->eventMouseMoved(pos);
    }
}

void Game::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    nuklear.eventMousePressed(pos, button);
    if (view) {
        view->eventMousePressed(pos, button);
    }
}

void Game::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    nuklear.eventMouseReleased(pos, button);
    if (view) {
        view->eventMouseReleased(pos, button);
    }
}

void Game::eventMouseScroll(const int xscroll, const int yscroll) {
    nuklear.eventMouseScroll(xscroll, yscroll);
    if (view) {
        view->eventMouseScroll(xscroll, yscroll);
    }
}

void Game::eventKeyPressed(const Key key, const Modifiers modifiers) {
    nuklear.eventKeyPressed(key, modifiers);
    if (view) {
        view->eventKeyPressed(key, modifiers);
    }

    if (config.input.galaxyMapToggle(key, modifiers)) {
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
    }
}

void Game::eventKeyReleased(const Key key, const Modifiers modifiers) {
    nuklear.eventKeyReleased(key, modifiers);
    if (view) {
        view->eventKeyReleased(key, modifiers);
    }
}

void Game::eventCharTyped(const uint32_t code) {
    nuklear.eventCharTyped(code);
    if (view) {
        view->eventCharTyped(code);
    }
}
