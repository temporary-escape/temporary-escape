#include "view_build.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ViewBuild::Gui::Gui(const Config& config, AssetsManager& assetsManager) :
    blockActionBar{config, assetsManager}, blockSelector{config}, blockSideMenu{config} {
}

ViewBuild::ViewBuild(const Config& config, Renderer& renderer, AssetsManager& assetsManager, Gui& gui) :
    config{config}, renderer{renderer}, assetsManager{assetsManager}, gui{gui}, scene{config} {

    gui.blockSelector.setBlocks(assetsManager.getBlocks().findAll());

    gui.blockSideMenu.setItems({
        GuiSideMenu::Item{
            assetsManager.getImages().find("icon_save"),
            "Save current ship",
            [this](bool active) {},
            false,
        },
        GuiSideMenu::Item{
            assetsManager.getImages().find("icon_open_folder"),
            "Open a ship to edit",
            [this](bool active) {},
            false,
        },
        GuiSideMenu::Item{
            assetsManager.getImages().find("icon_anticlockwise_rotation"),
            "Undo",
            [this](bool active) {},
            false,
        },
        GuiSideMenu::Item{
            assetsManager.getImages().find("icon_clockwise_rotation"),
            "Redo",
            [this](bool active) {},
            false,
        },
    });

    createScene();
    createEntityShip();
    createGridLines();
    createHelpers();
}

void ViewBuild::createScene() {
    auto entity = scene.createEntity();
    entity.addComponent<ComponentDirectionalLight>(Color4{2.0f, 1.9f, 1.8f, 1.0f});
    entity.addComponent<ComponentTransform>().translate(Vector3{3.0f, 1.0f, 3.0f});

    entity = scene.createEntity();
    auto& skybox = entity.addComponent<ComponentSkybox>(0);
    auto skyboxTextures = SkyboxTextures{renderer.getVulkan(), Color4{0.05f, 0.05f, 0.05f, 1.0f}};
    skybox.setTextures(renderer.getVulkan(), std::move(skyboxTextures));

    entity = scene.createEntity();
    auto& cameraTransform = entity.addComponent<ComponentTransform>();
    auto& cameraCamera = entity.addComponent<ComponentCamera>(cameraTransform);
    cameraCamera.setProjection(80.0f);
    cameraCamera.lookAt({3.0f, 3.0f, 3.0f}, {0.0f, 0.0f, 0.0f});
    scene.setPrimaryCamera(entity);
}

void ViewBuild::createGridLines() {
    static const int width{256};
    static const Color4 color{0.5f, 0.5f, 0.5f, 0.1f};
    static const Color4 colorX{1.0f, 0.25f, 0.25f, 0.3f};
    static const Color4 colorY{0.25f, 1.0f, 0.25f, 0.3f};
    static const Vector3 offset{0.5f, 0.0f, 0.5f};

    auto entity = scene.createEntity();
    auto& lines = entity.addComponent<ComponentLines>(std::vector<ComponentLines::Line>{});
    entity.addComponent<ComponentTransform>();

    for (int z = -width; z <= width; z++) {
        const auto start = Vector3{static_cast<float>(-width), 0.0f, static_cast<float>(z)} + offset;
        const auto end = Vector3{static_cast<float>(width), 0.0f, static_cast<float>(z)} + offset;

        lines.add(start, end, color);
    }

    for (int x = -width; x <= width; x++) {
        const auto start = Vector3{static_cast<float>(x), 0.0f, static_cast<float>(-width)} + offset;
        const auto end = Vector3{static_cast<float>(x), 0.0f, static_cast<float>(width)} + offset;

        lines.add(start, end, color);
    }

    {
        const Vector3 start{static_cast<float>(-width), 0.0f, 0.0f};
        const Vector3 end{static_cast<float>(width), 0.0f, 0.0f};
        lines.add(start, end, colorX);
    }

    {
        const Vector3 start{0.0f, 0.0f, static_cast<float>(-width)};
        const Vector3 end{0.0f, 0.0f, static_cast<float>(width)};
        lines.add(start, end, colorY);
    }
}

void ViewBuild::createHelpers() {
    entityHelperAdd = createHelperBox(Color4{0.0f, 1.0f, 0.0f, 1.0f}, 0.525f);
    entityHelperAdd.setDisabled(true);

    entityHelperRemove = createHelperBox(Color4{1.0f, 0.0f, 0.0f, 1.0f}, 0.525f);
    entityHelperRemove.setDisabled(true);
}

Entity ViewBuild::createHelperBox(const Color4& color, const float width) {
    auto entity = scene.createEntity();
    auto& lines = entity.addComponent<ComponentLines>(std::vector<ComponentLines::Line>{});
    entity.addComponent<ComponentTransform>();

    lines.add({width, -width, width}, {width, width, width}, color);
    lines.add({-width, -width, width}, {-width, width, width}, color);
    lines.add({-width, -width, -width}, {-width, width, -width}, color);
    lines.add({width, -width, -width}, {width, width, -width}, color);

    lines.add({width, -width, width}, {-width, -width, width}, color);
    lines.add({width, -width, width}, {width, -width, -width}, color);
    lines.add({-width, -width, -width}, {-width, -width, width}, color);
    lines.add({-width, -width, -width}, {width, -width, -width}, color);

    lines.add({width, width, width}, {-width, width, width}, color);
    lines.add({width, width, width}, {width, width, -width}, color);
    lines.add({-width, width, -width}, {-width, width, width}, color);
    lines.add({-width, width, -width}, {width, width, -width}, color);

    return entity;
}

void ViewBuild::createEntityShip() {
    entityShip = scene.createEntity();
    entityShip.addComponent<ComponentTransform>();
    auto& debug = entityShip.addComponent<ComponentDebug>();
    auto& grid = entityShip.addComponent<ComponentGrid>();
    grid.setDirty(true);

    auto block = assetsManager.getBlocks().find("block_crew_quarters_t1");
    grid.insert(Vector3i{0, 0, 0}, block, 0, 0, VoxelShape::Type::Cube);
    grid.insert(Vector3i{1, 0, 0}, block, 0, 0, VoxelShape::Type::Cube);
    grid.insert(Vector3i{0, 1, 0}, block, 0, 0, VoxelShape::Type::Cube);
    // grid.insert(Vector3i{0, 0, 1}, block, 0, 0, VoxelShape::Type::Cube);
}

void ViewBuild::addBlock() {
    if (!raycastResult) {
        return;
    }

    const auto actionBarItem = gui.blockActionBar.getActiveBlock();
    const auto actionBarColor = gui.blockActionBar.getActiveColor();
    if (!actionBarItem.block) {
        return;
    }

    auto& grid = entityShip.getComponent<ComponentGrid>();
    const auto pos = raycastResult->pos + raycastResult->orientation;
    logger.info("Inserting pos: {} hit: {} orientation: {} insert pos: {}",
                raycastResult->pos,
                raycastResult->hitPos,
                raycastResult->orientation,
                pos);
    grid.insert(pos, actionBarItem.block, 0, actionBarColor, actionBarItem.shape);
    grid.setDirty(true);
}

void ViewBuild::update(const float deltaTime) {
    scene.update(deltaTime);

    const auto [eyes, rayEnd] = scene.screenToWorld(raycastScreenPos, 16.0f);

    const auto& grid = entityShip.getComponent<ComponentGrid>();
    raycastResult = grid.rayCast(eyes, rayEnd);

    if (raycastResult) {
        if (entityHelperAdd.isDisabled()) {
            entityHelperAdd.setDisabled(false);
        }
        auto& transform = entityHelperAdd.getComponent<ComponentTransform>();
        transform.move(raycastResult->worldPos + raycastResult->normal);
    } else if (!entityHelperAdd.isDisabled()) {
        entityHelperAdd.setDisabled(true);
    }
}

void ViewBuild::renderCanvas(Canvas& canvas, const Vector2i& viewport) {
}

void ViewBuild::eventMouseMoved(const Vector2i& pos) {
    scene.eventMouseMoved(pos);

    raycastScreenPos = pos;
}

void ViewBuild::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    /*if (gui.contextMenu.isEnabled() && !gui.contextMenu.isCursorInside(pos)) {
        gui.contextMenu.setEnabled(false);
    }*/

    scene.eventMousePressed(pos, button);

    if (button == MouseButton::Left) {
        addBlock();
    }
}

void ViewBuild::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    scene.eventMouseReleased(pos, button);
}

void ViewBuild::eventMouseScroll(const int xscroll, const int yscroll) {
    if (yscroll > 0) {
        const auto nextIndex = (gui.blockActionBar.getActiveIndex() + 1) % gui.blockActionBar.getMaxItems();
        gui.blockActionBar.setActiveIndex(nextIndex);
    } else if (yscroll < 0) {
        auto nextIndex = static_cast<int>(gui.blockActionBar.getActiveIndex()) - 1;
        if (nextIndex < 0) {
            nextIndex = static_cast<int>(gui.blockActionBar.getMaxItems()) - 1;
        }
        gui.blockActionBar.setActiveIndex(nextIndex);
    }
}

void ViewBuild::eventKeyPressed(const Key key, const Modifiers modifiers) {
    scene.eventKeyPressed(key, modifiers);
}

void ViewBuild::eventKeyReleased(const Key key, const Modifiers modifiers) {
    scene.eventKeyReleased(key, modifiers);
}

void ViewBuild::eventCharTyped(const uint32_t code) {
    scene.eventCharTyped(code);
}

void ViewBuild::onEnter() {
    gui.blockSelector.setEnabled(true);
    gui.blockActionBar.setEnabled(true);
    gui.blockSideMenu.setEnabled(true);
}

void ViewBuild::onExit() {
    gui.blockSelector.setEnabled(false);
    gui.blockActionBar.setEnabled(false);
    gui.blockSideMenu.setEnabled(false);
}

Scene* ViewBuild::getScene() {
    return &scene;
}
