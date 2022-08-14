#include "ViewVoxelTest.hpp"
#include "../Assets/AssetManager.hpp"
#include "Client.hpp"
#include "Widgets.hpp"

#define CMP "ViewVoxelTest"

using namespace Engine;

ViewVoxelTest::ViewVoxelTest(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                             Widgets& widgets)
    : config(config), canvas(canvas), assetManager(assetManager), renderer(renderer), widgets(widgets) {

    scene = std::make_unique<Scene>();

    auto skybox = std::make_shared<Entity>();
    skybox->addComponent<ComponentSkybox>(Skybox::createOfColor(Color4{0.15f, 0.15f, 0.15f, 1.0f}));
    skybox->scale(Vector3{1000.0f});
    scene->addEntity(skybox);

    auto sun = std::make_shared<Entity>();
    sun->addComponent<ComponentDirectionalLight>(Color4{1.0f, 0.9f, 0.8f, 1.0f} * 1.0f);
    sun->translate(Vector3{2.0f, 2.0f, 2.0f});
    scene->addEntity(sun);

    auto camera = std::make_shared<Entity>();
    auto cmp = camera->addComponent<ComponentCameraTurntable>();
    auto userInput = camera->addComponent<ComponentUserInput>(*cmp);
    cmp->setProjection(70.0f);
    cmp->setZoom(5.0f);
    scene->addEntity(camera);
    scene->setPrimaryCamera(camera);

    /*auto asteroidModel = assetManager.find<AssetModel>("model_asteroid_01_a");

    for (auto y = 0; y < 24; y++) {
        for (auto x = 0; x < 24; x++) {
            auto asteroid = std::make_shared<Entity>();
            asteroid->addComponent<ComponentModel>(asteroidModel);
            asteroid->translate(Vector3{x * 3.0f, -3.0f, y * 3.0f});
            asteroid->updateTransform(asteroid->getTransform() * Grid::getRotationMatrix(x));
            scene->addEntity(asteroid);
        }
    }*/

    /*auto cubeDebugModel = assetManager.find<AssetModel>("debug_cube");
    for (auto y = 0; y < 24; y++) {
        for (auto x = 0; x < 4; x++) {
            auto asteroid = std::make_shared<Entity>();
            asteroid->addComponent<ComponentModel>(cubeDebugModel);
            asteroid->translate(Vector3{x + 6.0f, 0.0f, y});
            asteroid->updateTransform(asteroid->getTransform() * Grid::getRotationMatrix(y));
            scene->addEntity(asteroid);
        }
    }*/

    auto ship = std::make_shared<Entity>();
    auto grid = ship->addComponent<ComponentGrid>();
    grid->setDirty(true);

    auto block = assetManager.find<AssetBlock>("block_hull_t1");
    /*for (auto y = 0; y < 1; y++) {
        for (auto x = -20; x < 20; x++) {
            for (auto z = 0; z < 1; z++) {
                grid->insert(Vector3i{x, y, z}, block, 0, 0, 0);
            }
        }
    }*/

    for (auto s = 0; s < 24; s++) {
        for (auto x = 0; x < 4; x++) {
            for (auto y = 0; y < 1; y++) {
                grid->insert(Vector3i{x, y, s}, block, s, 0, x);
            }
        }
    }

    scene->addEntity(ship);
}

void ViewVoxelTest::render(const Vector2i& viewport) {
    if (scene != nullptr) {
        const auto now = std::chrono::steady_clock::now();
        auto timeDiff = now - lastTimePoint;
        lastTimePoint = now;
        if (timeDiff > std::chrono::milliseconds(100)) {
            timeDiff = std::chrono::milliseconds(100);
        }
        const auto delta = std::chrono::duration_cast<std::chrono::microseconds>(timeDiff).count() / 1000000.0f;
        scene->update(delta);

        renderer.setEnableBackground(true);
        renderer.setEnableBloom(true);
        renderer.render(*scene);
    }
}

void ViewVoxelTest::renderGui(const Vector2i& viewport) {
    /*canvas.beginPath();
    canvas.fillColor(Color4{1.0f, 0.0f, 0.0f, 0.2f});
    canvas.rect(Vector2{50.0f}, Vector2{100.0f});
    canvas.fill();
    canvas.closePath();*/
}

void ViewVoxelTest::eventMouseMoved(const Vector2i& pos) {
    if (scene) {
        scene->eventMouseMoved(pos);
    }
}

void ViewVoxelTest::eventMousePressed(const Vector2i& pos, MouseButton button) {
    if (scene) {
        scene->eventMousePressed(pos, button);
    }
}

void ViewVoxelTest::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    if (scene) {
        scene->eventMouseReleased(pos, button);
    }
}

void ViewVoxelTest::eventMouseScroll(const int xscroll, const int yscroll) {
    if (scene) {
        scene->eventMouseScroll(xscroll, yscroll);
    }
}

void ViewVoxelTest::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (scene) {
        scene->eventKeyPressed(key, modifiers);
    }
}

void ViewVoxelTest::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (scene) {
        scene->eventKeyReleased(key, modifiers);
    }
}
