#include "ViewBuild.hpp"

#include "../Scene/ComponentGrid.hpp"
#include "../Scene/ComponentModel.hpp"
#include "../Scene/ComponentWireframe.hpp"
#include "Messages.hpp"

using namespace Scissio;

ViewBuild::ViewBuild(const Config& config, Network::Client& client, Store& store, AssetManager& assetManager)
    : Store::Listener(store), config(config), client(client), assetManager(assetManager), mode{Mode::None},
      cameraMove{false}, cameraRotation{0, 0}, cameraRotate{false}, loading{true}, placeRotation{0} {

    camera.translate({0.0f, 0.0f, 2.0f});

    ship = scene.addEntity();
    auto block = assetManager.find<Model>("model_block_01_cube");
    Grid::BlockRef ref{block};
    auto grid = ship->addComponent<ComponentGrid>();
    grid->insert(ref, {0, 0, 0}, 0);
    grid->insert(ref, {1, 0, 0}, 0);
    grid->insert(ref, {2, 0, 0}, 0);
    ship->translate({0.0f, 0.0f, 0.0f});

    preview = scene.addEntity();
    preview->addComponent<ComponentModel>(block);
    preview->addComponent<ComponentWireframe>(WireframeModel::Box, Color4{0.0f, 1.0f, 0.0f, 1.0f});

    highlight = scene.addEntity();
    highlight->addComponent<ComponentWireframe>(WireframeModel::Box, Color4{1.0f, 1.0f, 0.0f, 1.0f});

    onNotify(store.blocks, [this](Store& store) {
        Log::d("All player usable blocks received, total: {}", store.blocks.value().size());
        (void)store;
        loading = false;

        const auto thumbnailDefault = this->assetManager.find<Image>("default-block-thumbnail");

        std::map<std::string, std::vector<const BlockDto*>> categories;

        for (const auto& block : store.blocks.value()) {
            categories[block.category].push_back(&block);
        }

        for (const auto& pair : categories) {
            blockSelector.categories.emplace_back();
            auto& category = blockSelector.categories.back();

            category.label = pair.first;

            for (const auto block : pair.second) {
                category.items.emplace_back();
                auto& item = category.items.back();

                const auto thumbnail = store.thumbnails.value().find(block->key);

                item.block = block;
                item.thumbnail = thumbnail != store.thumbnails.value().end() ? thumbnail->second : thumbnailDefault;
            }
        }
    });

    store.blocks.value().clear();
    client.send(0, MessageBlocksRequest{});

    sidebar = {
        {assetManager.find<Icon>("icons-arrow-cursor"), "Select", [this]() { mode = Mode::Select; }},
        {assetManager.find<Icon>("icons-info"), "Information", [this]() { mode = Mode::None; }},
        {assetManager.find<Icon>("icons-cube"), "Choose Block", [this]() { mode = Mode::Build; }},
        {assetManager.find<Icon>("icons-paint-roller"), "Coloring", [this]() { mode = Mode::None; }},
        {assetManager.find<Icon>("icons-anticlockwise-rotation"), "Undo", [this]() { actionUndo(); }},
        {assetManager.find<Icon>("icons-clockwise-rotation"), "Redo", [this]() { actionRedo(); }},
    };
}

void ViewBuild::update(const Vector2i& viewport) {
    this->viewport = viewport;

    {
        static constexpr auto d = 0.1f;

        Vector3 dir{0.0f};
        if (cameraMove[0]) {
            dir += Vector3{0.0f, 0.0f, -d};
        }
        if (cameraMove[1]) {
            dir += Vector3{-d, 0.0f, 0.0f};
        }
        if (cameraMove[2]) {
            dir += Vector3{0.0f, 0.0f, d};
        }
        if (cameraMove[3]) {
            dir += Vector3{d, 0.0f, 0.0f};
        }
        if (cameraMove[4]) {
            dir += Vector3{0.0f, d, 0.0f};
        }
        if (cameraMove[5]) {
            dir += Vector3{0.0f, -d, 0.0f};
        }

        const auto translation = Vector3(camera.getTransform()[3]);

        glm::mat4x4 transform{1.0f};
        transform = glm::rotate(transform, glm::radians(cameraRotation.x), Vector3{0.0f, 1.0f, 0.0f});
        transform = glm::rotate(transform, glm::radians(cameraRotation.y), Vector3{1.0f, 0.0f, 0.0f});

        dir = Vector3(transform * Vector4(dir, 1.0f));

        transform = glm::translate(glm::mat4x4{1.0f}, translation + dir) * transform;
        camera.updateTransform(transform);
        camera.setProjection(viewport, 70.0f);
    }

    {
        // const auto to = camera.getEyesPos() + camera.screenToWorld(viewport, mousePos) * 100.0f;
        // rayCastResult = ship->getComponent<ComponentGrid>()->rayCast(camera.getEyesPos(), to);

        if (placePosition.has_value() && mode == Mode::Build) {
            preview->updateTransform(Grid::ORIENTATIONS[placeRotation]);
            preview->move(placePosition.value());

        } else {
            preview->move(Vector3{-99999.0f});
        }

        if (mode != Mode::Select) {
            selectedBlock = std::nullopt;
            highlight->move(Vector3{-99999.0f});
        }
    }

    scene.update();
}

void ViewBuild::render(const Vector2i& viewport, Renderer& renderer) {
    if (loading) {
        return;
    }

    renderer.setView(camera.getViewMatrix());
    renderer.setProjection(camera.getProjectionMatrix());
    renderer.render(scene);
}

void ViewBuild::renderCanvas(const Vector2i& viewport, Canvas2D& canvas, GuiContext& gui) {
    if (loading) {
        Widgets::modal(gui, "Please wait!", "Initializing build mode...");
        return;
    }

    Widgets::sidebar(gui, sidebar);

    if (sidebar.at(2).active) {
        Widgets::blockSelector(gui, blockSelector, [this](const BlockDto& block) {
            try {
                preview->getComponent<ComponentModel>()->setModel(block.model);
            } catch (...) {
                EXCEPTION_NESTED("Failed to set preview model");
            }
        });
    }
}

void ViewBuild::eventMouseMoved(const Vector2i& pos) {
    this->mousePos = pos;

    if (cameraRotate) {
        cameraRotation += (mousePosOld - Vector2(pos)) * 0.2f;
        while (cameraRotation.x > 360.0f) {
            cameraRotation.x -= 360.0f;
        }
        while (cameraRotation.x < 0.0f) {
            cameraRotation.x += 360.0f;
        }
        cameraRotation.y = glm::clamp(cameraRotation.y, -90.0f, 90.0f);
        mousePosOld = pos;
    } else {
        calculateRayCast();
    }
}

void ViewBuild::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    mousePosOld = pos;
    cameraRotate |= button == MouseButton::Right;

    if (button == MouseButton::Left && rayCastResult.has_value()) {
        switch (mode) {
        case Mode::Build: {
            actionPlaceBlock();
            calculateRayCast();
            break;
        }
        case Mode::Select: {
            highlight->move(rayCastResult.value().node.get().data.pos);
            selectedBlock = rayCastResult.value().node;
            break;
        }
        default: {
            break;
        }
        }
    }
}

void ViewBuild::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    mousePosOld = pos;
    cameraRotate &= button != MouseButton::Right;
}

void ViewBuild::eventKeyPressed(const Key key, const Modifiers modifiers) {
    cameraMove[0] |= key == Key::LetterW;
    cameraMove[1] |= key == Key::LetterA;
    cameraMove[2] |= key == Key::LetterS;
    cameraMove[3] |= key == Key::LetterD;
    cameraMove[4] |= key == Key::SpaceBar;
    cameraMove[5] |= key == Key::LeftControl;

    if (key == Key::Delete && mode == Mode::Select) {
        actionRemoveBlock();
        highlight->move(Vector3{-99999.0f});
        selectedBlock = std::nullopt;
    }
}

void ViewBuild::eventKeyReleased(const Key key, const Modifiers modifiers) {
    cameraMove[0] &= key != Key::LetterW;
    cameraMove[1] &= key != Key::LetterA;
    cameraMove[2] &= key != Key::LetterS;
    cameraMove[3] &= key != Key::LetterD;
    cameraMove[4] &= key != Key::SpaceBar;
    cameraMove[5] &= key != Key::LeftControl;
}

void ViewBuild::eventMouseScroll(int xscroll, int yscroll) {
    placeRotation += yscroll;
    if (placeRotation < 0) {
        placeRotation = Grid::ORIENTATIONS.size() - 1;
    }
    if (placeRotation >= static_cast<int>(Grid::ORIENTATIONS.size())) {
        placeRotation = 0;
    }
}

void ViewBuild::calculateRayCast() {
    const auto to = camera.getEyesPos() + camera.screenToWorld(viewport, mousePos) * 100.0f;
    rayCastResult = ship->getComponent<ComponentGrid>()->rayCast(camera.getEyesPos(), to);

    if (rayCastResult.has_value()) {
        const auto& r = rayCastResult.value();
        placePosition = {r.node.get().data.pos + r.normal};
    } else {
        placePosition.reset();
    }
}

void ViewBuild::actionPlaceBlock() {
    if (!placePosition) {
        return;
    }

    Grid::BlockRef ref{preview->getComponent<ComponentModel>()->getModel()};
    auto grid = ship->getComponent<ComponentGrid>();
    const auto pos = placePosition.value();

    grid->insert(ref, pos, placeRotation);
    grid->debugTree();

    ActionPlaceBlock a;
    const auto& node = selectedBlock.value().get();
    a.ref = Grid::BlockRef{preview->getComponent<ComponentModel>()->getModel()};
    a.pos = node.data.pos;
    a.rot = node.data.rot;

    action(a);
}

void ViewBuild::actionRemoveBlock() {
    if (!selectedBlock) {
        return;
    }

    ActionRemoveBlock a;
    const auto& node = selectedBlock.value().get();
    a.pos = node.data.pos;
    a.rot = node.data.rot;

    action(a);
}

void ViewBuild::actionUndo() {
}

void ViewBuild::actionRedo() {
}

void ViewBuild::action(const Action& action) {
    std::visit(
        [this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            auto grid = ship->getComponent<ComponentGrid>();

            if constexpr (std::is_same_v<ActionPlaceBlock, T>) {
                const ActionPlaceBlock& action = arg;
                grid->insert(action.ref, action.pos, action.rot);
            } else if constexpr (std::is_same_v<ActionRemoveBlock, T>) {
                const ActionRemoveBlock& action = arg;
                const auto found = grid->find(action.pos);
                if (found) {
                    grid->remove(found.value().node);
                }
            }
        },
        action);
}
