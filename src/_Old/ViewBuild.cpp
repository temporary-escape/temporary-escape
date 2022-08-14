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

    actionBar.items.resize(10);
    for (size_t i = 0; i < actionBar.items.size(); i++) {
        auto& item = actionBar.items.at(i);

        item.onClick = [this, i]() { setActionBarActive(i); };

        if (!actionBar.blocks[i]) {
            item.image = images.noThumbnail;
            item.tooltip = "Click to browse blocks";
        }

        item.onDrop = [this, i](const std::any& drop) {
            if (drop.type() == typeid(AssetBlockShape)) {
                const auto& value = std::any_cast<AssetBlockShape>(drop);
                setActionBarItem(i, value.block, value.shape);
            }
        };
    }

    setActionBarItem(0, assetManager.find<AssetBlock>("block_hull_t1"), Shape::Type::Cube);
    setActionBarActive(0);
}

void ViewBuild::setActionBarItem(size_t index, const AssetBlockPtr& block, Shape::Type shape) {
    auto& item = actionBar.items.at(index);

    item.image = block->getImageForShape(shape);
    item.onClick = nullptr;
    item.tooltip = fmt::format("{} ({})", block->getTitle(), shapeTypeToFriendlyName(shape));

    actionBar.blocks[index] = block;
    actionBar.shapes[index] = shape;
}

void ViewBuild::setActionBarActive(size_t index) {
    actionBar.active = index;

    if (!actionBar.blocks[index]) {
        blockBrowser.enabled = true;
    }
}

void ViewBuild::load() {
    loading = true;

    entityHelperAdd.reset();
    entityShip.reset();
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

    entityShip = std::make_shared<Entity>();
    auto grid = entityShip->addComponent<ComponentGrid>();
    grid->setDirty(true);

    auto block = assetManager.find<AssetBlock>("block_hull_t1");
    grid->insert(Vector3i{0, 0, 0}, block, 0, 0, Shape::Type::Cube);
    grid->insert(Vector3i{-1, 0, 0}, block, 0, 0, Shape::Type::Cube);
    grid->dump();
    // grid->insert(Vector3i{0, 1, 0}, block, 0, 0, Shape::Type::Cube);
    // grid->insert(Vector3i{-1, 1, 0}, block, 0, 0, Shape::Type::Cube);

    scene->addEntity(entityShip);

    const auto color = Color4{0.0f, 1.0f, 0.0f, 1.0f};
    std::vector<ComponentLines::Line> helperLines = {
        {{-1.0, -1.0, -1.0}, color, {+1.0, -1.0, -1.0}, color}, {{+1.0, -1.0, -1.0}, color, {+1.0, -1.0, +1.0}, color},
        {{+1.0, -1.0, +1.0}, color, {-1.0, -1.0, +1.0}, color}, {{-1.0, -1.0, +1.0}, color, {-1.0, -1.0, -1.0}, color},
        {{-1.0, +1.0, -1.0}, color, {+1.0, +1.0, -1.0}, color}, {{+1.0, +1.0, -1.0}, color, {+1.0, +1.0, +1.0}, color},
        {{+1.0, +1.0, +1.0}, color, {-1.0, +1.0, +1.0}, color}, {{-1.0, +1.0, +1.0}, color, {-1.0, +1.0, -1.0}, color},
        {{-1.0, +1.0, -1.0}, color, {-1.0, -1.0, -1.0}, color}, {{+1.0, +1.0, -1.0}, color, {+1.0, -1.0, -1.0}, color},
        {{+1.0, +1.0, +1.0}, color, {+1.0, -1.0, +1.0}, color}, {{-1.0, +1.0, +1.0}, color, {-1.0, -1.0, +1.0}, color},
    };

    entityHelperAdd = std::make_shared<Entity>();
    entityHelperAdd->addComponent<ComponentLines>(std::move(helperLines));
    entityHelperAdd->scale(Vector3{0.98f, 0.98f, 0.98f} * Vector3{0.5f});
    entityHelperAdd->setVisible(false);

    scene->addEntity(entityHelperAdd);

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

        const auto grid = entityShip->getComponent<ComponentGrid>();
        const auto camera = scene->getPrimaryCamera();
        const auto ray = camera->screenToWorld(mousePos) * 100.0f;
        rayCastResult = grid->rayCast(camera->getEyesPos(), ray);

        if (rayCastResult) {
            entityHelperAdd->setVisible(true);
            entityHelperAdd->move(rayCastResult->worldPos + rayCastResult->normal);
        } else {
            entityHelperAdd->setVisible(false);
        }

        renderer.setEnableBackground(true);
        renderer.setEnableBloom(true);
        renderer.render(*scene);
    }
}

void ViewBuild::renderGui(const Vector2i& viewport) {
    if (loading) {
        widgets.loading("Initializing build data...", 0.9f);
    } else {
        widgets.actionBar(64.0f, actionBar.items);
        if (blockBrowser.enabled) {
            widgets.blockBrowser(blockBrowser.data);
        }
    }
}

void ViewBuild::actionPlaceBlock() {
    const auto& block = actionBar.blocks[actionBar.active];
    const auto shape = actionBar.shapes[actionBar.active];

    if (!rayCastResult || !block) {
        return;
    }

    const auto grid = entityShip->getComponent<ComponentGrid>();
    Log::d(CMP, "rayCastResult pos: {} normal: {} world: {} hit: {}", rayCastResult->pos, rayCastResult->normal,
           rayCastResult->worldPos, rayCastResult->hitPos);
    const auto pos = Vector3i{rayCastResult->worldPos} + Vector3i{rayCastResult->normal};
    grid->insert(pos, block, 0, 0, shape);
    grid->setDirty(true);
    grid->dump();
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

        blockBrowser.data.categories.clear();
        blockBrowser.data.categories.emplace_back();
        blockBrowser.data.onHover = [this](const Widgets::BlockBrowserItem* item) {
            if (item == nullptr) {
                blockBrowser.hovered = nullptr;
            } else {
                blockBrowser.hovered = item->block;
            }
        };
        blockBrowser.data.onClose = [this]() { blockBrowser.enabled = false; };

        auto& allCategory = blockBrowser.data.categories.front();
        allCategory.label = "All Blocks";
        allCategory.shown = true;

        const auto setItem = [&](Widgets::BlockBrowserItem& item, const AssetBlockPtr& block, const Shape::Type shape) {
            item.image = block->getImageForShape(shape);
            item.label = fmt::format("{} ({})", block->getTitle(), shapeTypeToFriendlyName(shape));
            item.block = block;
            item.drag = AssetBlockShape{block, shape};
            // item.onClick = [this, block, shape]() { setActionBarActiveItem(block, shape); };
        };

        for (const auto& tuple : blockBrowser.available) {
            const auto& block = std::get<0>(tuple);
            const auto& shape = std::get<1>(tuple);

            allCategory.items.emplace_back();
            auto& item = allCategory.items.back();
            setItem(item, block, shape);
        }

        for (const auto& tuple : blockBrowser.available) {
            const auto& block = std::get<0>(tuple);
            const auto& shape = std::get<1>(tuple);

            auto it = std::find_if(
                blockBrowser.data.categories.begin(), blockBrowser.data.categories.end(),
                [&](Widgets::BlockBrowserCategory& category) { return category.label == block->getCategory(); });

            if (it == blockBrowser.data.categories.end()) {
                blockBrowser.data.categories.emplace_back();
                auto& category = blockBrowser.data.categories.back();
                category.label = block->getCategory();
                category.items.emplace_back();
                setItem(category.items.back(), block, shape);
            } else {
                it->items.emplace_back();
                setItem(it->items.back(), block, shape);
            }
        }

        std::sort(blockBrowser.data.categories.begin() + 1, blockBrowser.data.categories.end(),
                  [](const auto& a, const auto& b) -> bool { return a.label.compare(b.label) > 0; });
    });
}

void ViewBuild::eventMouseMoved(const Vector2i& pos) {
    if (!widgets.inputOverlap(pos)) {
        mousePos = pos;
        scene->eventMouseMoved(pos);
    }
}

void ViewBuild::eventMousePressed(const Vector2i& pos, MouseButton button) {
    if (blockBrowser.enabled && blockBrowser.hovered && button == MouseButton::Left) {
        blockBrowser.dragging = blockBrowser.hovered;
    }
    if (!widgets.inputOverlap(pos)) {
        scene->eventMousePressed(pos, button);
        if (button == MouseButton::Left) {
            actionPlaceBlock();
        }
    }
}

void ViewBuild::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    if (blockBrowser.enabled && blockBrowser.dragging) {
        blockBrowser.dragging = nullptr;
    }
    if (!widgets.inputOverlap(pos)) {
        scene->eventMouseReleased(pos, button);
    }
}

void ViewBuild::eventMouseScroll(const int xscroll, const int yscroll) {
    if (!widgets.inputOverlap(mousePos)) {
        scene->eventMouseScroll(xscroll, yscroll);
    }
}

void ViewBuild::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (!widgets.inputOverlap(mousePos)) {
        scene->eventKeyPressed(key, modifiers);
    }
}

void ViewBuild::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (!widgets.inputOverlap(mousePos)) {
        scene->eventKeyReleased(key, modifiers);
    }
}
