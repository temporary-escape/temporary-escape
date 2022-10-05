#include "Game.hpp"
#include "../Database/RocksDB.hpp"
#include "../Server/Server.hpp"
#include "../Utils/Random.hpp"
#include "Client.hpp"

#define CMP "Game"

using namespace Engine;

Game::Game(const Config& config, VulkanDevice& vulkan, Registry& registry, Canvas& canvas, FontFamily& font,
           Scene::Pipelines& scenePipelines, Status& status) :
    config{config},
    vulkan{vulkan},
    registry{registry},
    canvas{canvas},
    font{font},
    scenePipelines{scenePipelines},
    status{status},
    userInput{config, *this},
    nuklear{canvas, font.regular, 19.0f} {
}

Game::~Game() {
}

void Game::update(float deltaTime) {
    /*if (!viewBuild) {
        viewBuild = std::make_unique<ViewBuild>(config, vulkan, scenePipelines, registry, canvas, font, nuklear);
        view = viewBuild.get();
    }*/
    if (client && clientLoad.valid() && clientLoad.ready()) {
        Log::i(CMP, "Starting scene");
        status.message = "Entering universe...";
        status.value = 1.0f;

        clientLoad.get();
    }

    if (server && serverLoad.valid() && serverLoad.ready()) {
        Log::i(CMP, "Starting client");
        serverLoad.get();

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
        clientLoad = client->connect("localhost", config.serverPort);
    }

    if (db && !server) {
        Log::i(CMP, "Starting server");
        status.message = "Starting server...";
        status.value = 0.8f;

        server = std::make_unique<Server>(config, registry, *db);
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
    canvas.begin(viewport);

    if (view) {
        view->renderCanvas(viewport);
    }

    /*nuklear.begin(viewport);

    const auto flags = Nuklear::WindowFlags::Border | Nuklear::WindowFlags::Background | Nuklear::WindowFlags::Title;
    if (nuklear.beginWindow("Hello World!", {200.0f, 200.0f}, {500.0f, 500.0f}, flags)) {
        nuklear.layoutDynamic(30.0f, 1);
        nuklear.button("Click me");
        nuklear.endWindow();
    }

    nuklear.end();*/
    canvas.end();
}

const Skybox* Game::getSkybox() {
    if (view) {
        return view->getSkybox();
    }
    return nullptr;
}

void Game::eventUserInput(const UserInput::Event& event) {
    if (view) {
        view->eventUserInput(event);
    }
}

void Game::eventMouseMoved(const Vector2i& pos) {
    nuklear.eventMouseMoved(pos);
    userInput.eventMouseMoved(pos);
}

void Game::eventMousePressed(const Vector2i& pos, MouseButton button) {
    nuklear.eventMousePressed(pos, button);
    userInput.eventMousePressed(pos, button);
}

void Game::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    nuklear.eventMouseReleased(pos, button);
    userInput.eventMouseReleased(pos, button);
}

void Game::eventMouseScroll(int xscroll, int yscroll) {
    nuklear.eventMouseScroll(xscroll, yscroll);
    userInput.eventMouseScroll(xscroll, yscroll);
}

void Game::eventKeyPressed(Key key, Modifiers modifiers) {
    nuklear.eventKeyPressed(key, modifiers);
    userInput.eventKeyPressed(key, modifiers);
}

void Game::eventKeyReleased(Key key, Modifiers modifiers) {
    nuklear.eventKeyReleased(key, modifiers);
    userInput.eventKeyReleased(key, modifiers);
}

void Game::eventWindowResized(const Vector2i& size) {
}

void Game::eventCharTyped(uint32_t code) {
    nuklear.eventCharTyped(code);
}
