#include "view_build.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ViewBuild::ViewBuild(const Config& config, VulkanRenderer& vulkan, AssetsManager& assetsManager,
                     VoxelShapeCache& voxelShapeCache) :
    config{config},
    vulkan{vulkan},
    assetsManager{assetsManager},
    guiBlockSelector{config, assetsManager},
    guiBlockInfo{config, assetsManager},
    scene{config, &voxelShapeCache} {

    guiBlockSelector.setBlocks(assetsManager.getBlocks().findAll());
    guiBlockInfo.setEnabled(false);

    createScene();
    createEntityShip();
    createGridLines();
    createHelpers();

    selected.block = guiBlockSelector.getSelectedBlock();
    selected.color = guiBlockSelector.getSelectedColor();
    selected.shape = guiBlockSelector.getSelectedShape();
    updateSelectedBlock();
}

void ViewBuild::createScene() {
    auto entity = scene.createEntity();
    entity.addComponent<ComponentDirectionalLight>(Color4{2.0f, 1.9f, 1.8f, 1.0f});
    entity.addComponent<ComponentTransform>().translate(Vector3{3.0f, 2.0f, 3.0f});

    entity = scene.createEntity();
    auto& skybox = entity.addComponent<ComponentSkybox>(0);
    auto skyboxTextures = SkyboxTextures{vulkan, Color4{0.02f, 0.02f, 0.02f, 1.0f}};
    skybox.setTextures(vulkan, std::move(skyboxTextures));

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

    entityHelperAdd.addComponent<ComponentGrid>();

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
    // auto& debug = entityShip.addComponent<ComponentDebug>();
    auto& grid = entityShip.addComponent<ComponentGrid>();
    grid.setDirty(true);

    auto block = assetsManager.getBlocks().find("block_hull_t1");
    for (int z = -10; z <= 10; z++) {
        for (int x = -10; x <= 10; x++) {
            grid.insert(Vector3i{x, 0, z}, block, 0, 0, VoxelShape::Type::Cube);
        }
    }
}

void ViewBuild::addBlock() {
    if (!raycastResult || !selected.block) {
        return;
    }

    const auto pos = raycastResult->pos + raycastResult->orientation;
    auto& grid = entityShip.getComponent<ComponentGrid>();
    grid.insert(pos, selected.block, currentRotation, selected.color, selected.shape);
    grid.setDirty(true);
}

void ViewBuild::removeBlock() {
    if (!raycastResult) {
        return;
    }
}

void ViewBuild::updateSelectedBlock() {
    auto& grid = entityHelperAdd.getComponent<ComponentGrid>();
    if (selected.block) {
        entityHelperAdd.setDisabled(false);
        grid.insert({0, 0, 0}, selected.block, currentRotation, selected.color, selected.shape);
        grid.setDirty(true);
    } else {
        entityHelperAdd.setDisabled(true);
    }
}

void ViewBuild::update(const float deltaTime) {
    scene.update(deltaTime);

    if (guiBlockSelector.getSelectedBlock() != selected.block ||
        guiBlockSelector.getSelectedColor() != selected.color ||
        guiBlockSelector.getSelectedShape() != selected.shape || currentRotation != selected.rotation) {

        selected.block = guiBlockSelector.getSelectedBlock();
        selected.color = guiBlockSelector.getSelectedColor();
        selected.shape = guiBlockSelector.getSelectedShape();
        selected.rotation = currentRotation;

        updateSelectedBlock();
    }

    if (const auto hoveredBlock = guiBlockSelector.getHoveredBlock(); hoveredBlock) {
        guiBlockInfo.setBlock(hoveredBlock);
        guiBlockInfo.setShape(guiBlockSelector.getHoveredShape());
        guiBlockInfo.setEnabled(true);

        Vector2 offset{guiBlockSelector.getSize().x / -2.0f, 0.0f};
        offset += guiBlockSelector.getHoveredOffset();
        guiBlockInfo.setOffset(offset);
    } else {
        guiBlockInfo.setEnabled(false);
    }

    const auto [eyes, rayEnd] = scene.screenToWorld(raycastScreenPos, 16.0f);

    const auto& grid = entityShip.getComponent<ComponentGrid>();
    raycastResult = grid.rayCast(eyes, rayEnd);

    if (raycastResult) {
        // Action to add new blocks to the grid
        if (guiBlockSelector.getAction() == GuiBlockSelector::Action::Add) {
            if (entityHelperAdd.isDisabled()) {
                entityHelperAdd.setDisabled(false);
            }
            if (!entityHelperRemove.isDisabled()) {
                entityHelperRemove.setDisabled(true);
            }

            auto& transform = entityHelperAdd.getComponent<ComponentTransform>();
            transform.move(raycastResult->worldPos + raycastResult->normal);
        }
        // Action to remove blocks from the grid
        else if (guiBlockSelector.getAction() == GuiBlockSelector::Action::Remove) {
            if (!entityHelperAdd.isDisabled()) {
                entityHelperAdd.setDisabled(true);
            }
            if (entityHelperRemove.isDisabled()) {
                entityHelperRemove.setDisabled(false);
            }

            auto& transform = entityHelperRemove.getComponent<ComponentTransform>();
            transform.move(raycastResult->worldPos);
        }
        // Replace blocks
        else if (guiBlockSelector.getAction() == GuiBlockSelector::Action::Replace) {
            if (!entityHelperAdd.isDisabled()) {
                entityHelperAdd.setDisabled(true);
            }
            if (!entityHelperRemove.isDisabled()) {
                entityHelperRemove.setDisabled(true);
            }
        }
    } else {
        if (!entityHelperAdd.isDisabled()) {
            entityHelperAdd.setDisabled(true);
        }
        if (!entityHelperRemove.isDisabled()) {
            entityHelperRemove.setDisabled(true);
        }
    }
}

void ViewBuild::renderCanvas(Canvas& canvas, const Vector2i& viewport) {
}

void ViewBuild::renderNuklear(Nuklear& nuklear, const Vector2i& viewport) {
    guiBlockSelector.draw(nuklear, viewport);
    guiBlockInfo.draw(nuklear, viewport);
}

void ViewBuild::eventMouseMoved(const Vector2i& pos) {
    scene.eventMouseMoved(pos);

    raycastScreenPos = pos;
}

void ViewBuild::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    scene.eventMousePressed(pos, button);

    if (button == MouseButton::Left) {
        if (guiBlockSelector.getAction() == GuiBlockSelector::Action::Add) {
            addBlock();
        } else if (guiBlockSelector.getAction() == GuiBlockSelector::Action::Remove) {
            removeBlock();
        }
    }
}

void ViewBuild::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    scene.eventMouseReleased(pos, button);
}

void ViewBuild::eventMouseScroll(const int xscroll, const int yscroll) {
    scene.eventMouseScroll(xscroll, yscroll);

    if (yscroll > 0) {
        if (currentRotation < 23) {
            ++currentRotation;
        } else {
            currentRotation = 0;
        }
    } else if (yscroll < 0) {
        if (currentRotation > 0) {
            --currentRotation;
        } else {
            currentRotation = 23;
        }
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
    guiBlockSelector.setEnabled(true);
}

void ViewBuild::onExit() {
    guiBlockSelector.setEnabled(false);
}

Scene* ViewBuild::getScene() {
    return &scene;
}
