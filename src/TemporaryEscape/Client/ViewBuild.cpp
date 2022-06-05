#include "ViewBuild.hpp"
#include "../Assets/AssetManager.hpp"
#include "Client.hpp"
#include "Widgets.hpp"

#define CMP "ViewVoxelTest"

using namespace Engine;

ViewBuild::ViewBuild(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                     Client& client, Widgets& widgets)
    : config(config), canvas(canvas), assetManager(assetManager), renderer(renderer), client(client), widgets(widgets),
      loading(true) {

    images.noThumbnail = assetManager.find<AssetImage>("image_no_thumbnail");

    actionBar.blocks[0] = assetManager.find<AssetBlock>("block_hull_t1");
    actionBar.shapes[0] = Shape::Type::Penta;
}

void ViewBuild::load() {
    loading = true;

    scene = std::make_unique<Scene>();

    auto skybox = std::make_shared<Entity>();
    skybox->addComponent<ComponentSkybox>(Skybox::createOfColor(Color4{0.15f, 0.15f, 0.15f, 1.0f}));
    skybox->scale(Vector3{1000.0f});
    scene->addEntity(skybox);

    auto sun = std::make_shared<Entity>();
    sun->addComponent<ComponentDirectionalLight>(Color4{1.0f, 0.9f, 0.8f, 1.0f} * 3.0f);
    sun->translate(Vector3{-2.0f, 2.0f, 2.0f});
    scene->addEntity(sun);

    auto camera = std::make_shared<Entity>();
    auto cmp = camera->addComponent<ComponentCameraTurntable>();
    auto userInput = camera->addComponent<ComponentUserInput>(*cmp);
    cmp->setProjection(70.0f);
    cmp->setZoom(5.0f);
    scene->addEntity(camera);
    scene->setPrimaryCamera(camera);

    auto ship = std::make_shared<Entity>();
    auto grid = ship->addComponent<ComponentGrid>();
    grid->setDirty(true);

    auto block = assetManager.find<AssetBlock>("block_hull_t1");
    grid->insert(Vector3i{0, 0, 0}, block, 0, 0, Shape::Type::Cube);

    scene->addEntity(ship);

    fetchUnlockedBlocks();
}

void ViewBuild::render(const Vector2i& viewport) {
    if (scene) {
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

void ViewBuild::renderGui(const Vector2i& viewport) {
    if (loading) {
        widgets.loading("Initializing build data...", 0.9f);
    } else {
        renderActionBar();
        renderBlockBrowser();
    }
}

void ViewBuild::renderActionBar() {
    std::vector<Widgets::ActionBarItem> items;
    items.resize(10);

    for (size_t i = 0; i < items.size(); i++) {
        if (!actionBar.blocks[i]) {
            items[i].image = images.noThumbnail;
            items[i].onClick = [this]() { blockBrowser.enabled = true; };
            items[i].tooltip = "Click to browse blocks";
        } else {
            items[i].image = actionBar.blocks[i]->getImageForShape(actionBar.shapes[i]);
            items[i].onClick = nullptr;
            items[i].tooltip =
                fmt::format("{} ({})", actionBar.blocks[i]->getTitle(), shapeTypeToFriendlyName(actionBar.shapes[i]));
        }
    }

    widgets.actionBar(64.0f, items);
}

void ViewBuild::renderBlockBrowser() {
    if (!blockBrowser.enabled) {
        return;
    }

    std::vector<Widgets::BlockBrowserItem> items;

    for (const auto& [block, shape] : blockBrowser.available) {
        items.emplace_back();
        auto& item = items.back();

        item.image = block->getImageForShape(shape);
        item.label = fmt::format("{} ({})", block->getTitle(), shapeTypeToFriendlyName(shape));
        item.onHover = nullptr;
    }

    auto onClose = [this]() { blockBrowser.enabled = false; };

    widgets.blockBrowser(Vector2i{100.0f, 100.0f}, items, blockBrowser.filter, onClose);
}

void ViewBuild::fetchUnlockedBlocks() {
    MessageUnlockedBlocks::Request req{};
    client.send(req, [this](MessageUnlockedBlocks::Response res) {
        loading = false;
        // availableBlocks = std::move(res.blocks);
        blockBrowser.available.clear();

        for (const auto& block : res.blocks) {
            for (const auto& shapeType : block->getAllowedTypes()) {
                blockBrowser.available.emplace_back(block, shapeType);
            }
        }

        std::sort(blockBrowser.available.begin(), blockBrowser.available.end(),
                  [](const auto& a, const auto& b) -> bool {
                      return std::get<0>(a)->getTitle().compare(std::get<0>(b)->getTitle()) > 0;
                  });
    });
}

void ViewBuild::eventMouseMoved(const Vector2i& pos) {
    scene->eventMouseMoved(pos);
}

void ViewBuild::eventMousePressed(const Vector2i& pos, MouseButton button) {
    scene->eventMousePressed(pos, button);
}

void ViewBuild::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    scene->eventMouseReleased(pos, button);
}

void ViewBuild::eventMouseScroll(const int xscroll, const int yscroll) {
    scene->eventMouseScroll(xscroll, yscroll);
}

void ViewBuild::eventKeyPressed(const Key key, const Modifiers modifiers) {
    scene->eventKeyPressed(key, modifiers);
}

void ViewBuild::eventKeyReleased(const Key key, const Modifiers modifiers) {
    scene->eventKeyReleased(key, modifiers);
}
