#include "ViewBuild.hpp"
#include "../File/MsgpackFileReader.hpp"
#include "../File/MsgpackFileWriter.hpp"
#include "../File/TebFileHeader.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static Path getBuildDir(const Config& config) {
    return config.assetsPath / "base" / "ships";
}

ViewBuild::ViewBuild(const Config& config, VulkanRenderer& vulkan, GuiManager& guiManager, AudioContext& audio,
                     AssetsManager& assetsManager, VoxelShapeCache& voxelShapeCache, FontFamily& font) :
    config{config},
    vulkan{vulkan},
    assetsManager{assetsManager},
    uiAudioSource{audio.createSource()},
    scene{config, &voxelShapeCache} {

    scene.setSelectionEnabled(false);

    createScene();
    createEntityShip();
    createGridLines();
    createHelpers();

    // selected.block = guiBlockSelector.getSelectedBlock();
    // selected.color = guiBlockSelector.getSelectedColor();
    // selected.shape = guiBlockSelector.getSelectedShape();
    updateSelectedBlock();

    sound.build = assetsManager.getSounds().find("build_01");
    sound.destroy = assetsManager.getSounds().find("build_reverse_01");
}

void ViewBuild::createScene() {
    {
        auto entity = scene.createEntity();
        entity.addComponent<ComponentDirectionalLight>(Color4{2.0f, 1.9f, 1.8f, 1.0f});
        entity.addComponent<ComponentTransform>().translate(Vector3{3.0f, 2.0f, 3.0f});
    }

    {
        auto entity = scene.createEntity();
        auto& skybox = entity.addComponent<ComponentSkybox>(0);
        auto skyboxTextures = SkyboxTextures{vulkan, Color4{0.02f, 0.02f, 0.02f, 1.0f}};
        skybox.setTextures(vulkan, std::move(skyboxTextures));
    }

    {
        auto entity = scene.createEntity();
        auto& transform = entity.addComponent<ComponentTransform>();
        auto& camera = entity.addComponent<ComponentCamera>(transform);
        auto& cameraOrbital = entity.addComponent<ComponentCameraOrbital>(camera);
        camera.setProjection(80.0f);
        cameraOrbital.setDistance(5.0f);
        cameraOrbital.setTarget({0.0f, 0.0f, 0.0f});
        cameraOrbital.setRotation({-45.0f, 45.0f});
        scene.setPrimaryCamera(entity);
    }
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
    grid.setDirty();

    auto block = assetsManager.getBlocks().find("block_hull_t1");
    grid.insert(Vector3i{0, 0, 0}, block, 0, 0, VoxelShape::Type::Cube);
}

void ViewBuild::addBlock() {
    if (!raycastResult || !selected.block) {
        return;
    }

    const auto pos = raycastResult->pos + raycastResult->orientation;
    auto& grid = entityShip.getComponent<ComponentGrid>();
    grid.insert(pos, selected.block, currentRotation, selected.color, selected.shape);
    grid.setDirty();

    // Insert a new undo item
    if (historyPos < history.size()) {
        history.erase(history.begin() + historyPos, history.end());
    }
    auto& action = history.emplace_back();
    action.block = selected.block;
    action.pos = pos;
    action.rotation = currentRotation;
    action.color = selected.color;
    action.shape = selected.shape;
    action.added = true;

    if (history.size() > maxHistoryItems) {
        history.pop_front();
    }

    historyPos = static_cast<int64_t>(history.size());

    uiAudioSource.bind(sound.build->getAudioBuffer());
    uiAudioSource.play();
}

void ViewBuild::removeBlock() {
    if (!raycastResult) {
        return;
    }

    const auto pos = raycastResult->pos;
    auto& grid = entityShip.getComponent<ComponentGrid>();
    auto found = grid.find(pos);

    if (!found) {
        return;
    }

    // Insert a new undo item
    if (historyPos < history.size()) {
        history.erase(history.begin() + historyPos, history.end());
    }
    auto& action = history.emplace_back();
    action.block = grid.getType(found->type.value());
    action.pos = pos;
    action.rotation = found->rotation.value();
    action.color = found->color.value();
    action.shape = static_cast<VoxelShape::Type>(found->shape.value());
    action.added = false;

    if (history.size() > maxHistoryItems) {
        history.pop_front();
    }

    historyPos = static_cast<int64_t>(history.size());

    grid.remove(pos);
    grid.setDirty();

    uiAudioSource.bind(sound.destroy->getAudioBuffer());
    uiAudioSource.play();
}

void ViewBuild::doUndo() {
    if (historyPos > 0) {
        --historyPos;
    } else {
        return;
    }

    auto& grid = entityShip.getComponent<ComponentGrid>();

    auto& action = history.at(historyPos);
    if (action.added) {
        grid.remove(action.pos);
    } else {
        grid.insert(action.pos, action.block, action.rotation, action.color, action.shape);
    }

    grid.setDirty();
}

void ViewBuild::doRedo() {
    if (historyPos >= history.size()) {
        return;
    }

    auto& grid = entityShip.getComponent<ComponentGrid>();
    auto& action = history.at(historyPos);

    if (!action.added) {
        grid.remove(action.pos);
    } else {
        grid.insert(action.pos, action.block, action.rotation, action.color, action.shape);
    }

    ++historyPos;
    grid.setDirty();
}

void ViewBuild::doSave() {
    /*if (guiFileBrowser.isEnabled()) {
        return;
    }

    guiFileBrowser.setConfirmText("Save");
    guiFileBrowser.setFolder(getBuildDir(config), ".teb");
    guiFileBrowser.setEnabled(true);
    guiFileBrowser.setConfirmCallback([this](Path path) {
        try {
            resetHistory();

            logger.info("Saving current design to: {}", path);

            auto& grid = entityShip.getComponent<ComponentGrid>();
            MsgpackFileWriter file{path};
            TebFileHeader header{};
            header.type = TebFileType::Ship;
            file.pack(header);
            file.pack(grid);
        } catch (std::exception& e) {
            BACKTRACE(e, "Failed to save grid");
        }
    });*/
}

void ViewBuild::doLoad() {
    /*if (guiFileBrowser.isEnabled()) {
        return;
    }

    guiFileBrowser.setConfirmText("Load");
    guiFileBrowser.setFolder(getBuildDir(config), ".teb");
    guiFileBrowser.setEnabled(true);
    guiFileBrowser.setConfirmCallback([this](Path path) {
        try {
            resetHistory();

            logger.info("Loading design from: {}", path);

            scene.removeEntity(entityShip);
            entityShip = scene.createEntity();
            entityShip.addComponent<ComponentTransform>();
            auto& grid = entityShip.addComponent<ComponentGrid>();
            grid.setDirty(true);

            MsgpackFileReader file{path};
            TebFileHeader header{};
            file.unpack(header);

            if (header.type != TebFileType::Ship) {
                EXCEPTION("Bad file type");
            }

            file.unpack(grid);
        } catch (std::exception& e) {
            BACKTRACE(e, "Failed to save grid");
        }
    });*/
}

void ViewBuild::resetHistory() {
    historyPos = 0;
    history.clear();
}

void ViewBuild::updateSelectedBlock() {
    if (selected.block && selected.block->getRotationMode() == Block::RotationMode::None) {
        currentRotation = 0;
    }

    auto& grid = entityHelperAdd.getComponent<ComponentGrid>();
    if (selected.block) {
        entityHelperAdd.setDisabled(false);
        grid.insert({0, 0, 0}, selected.block, currentRotation, selected.color, selected.shape);
        grid.setDirty();
    } else {
        entityHelperAdd.setDisabled(true);
    }
}

void ViewBuild::update(const float deltaTime, const Vector2i& viewport) {
    scene.update(deltaTime);

    /*if (guiBlockSelector.getSelectedBlock() != selected.block ||
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
    }*/
}

void ViewBuild::renderCanvas(Canvas& canvas, const Vector2i& viewport) {
}

void ViewBuild::eventMouseMoved(const Vector2i& pos) {
    scene.eventMouseMoved(pos);

    raycastScreenPos = pos;
}

void ViewBuild::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    scene.eventMousePressed(pos, button);

    /*if (button == MouseButton::Left) {
        if (guiBlockSelector.getAction() == GuiBlockSelector::Action::Add) {
            addBlock();
        } else if (guiBlockSelector.getAction() == GuiBlockSelector::Action::Remove) {
            removeBlock();
        }
    }*/
}

void ViewBuild::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    scene.eventMouseReleased(pos, button);
}

void ViewBuild::eventMouseScroll(const int xscroll, const int yscroll) {
    scene.eventMouseScroll(xscroll, yscroll);
}

void ViewBuild::eventKeyPressed(const Key key, const Modifiers modifiers) {
    scene.eventKeyPressed(key, modifiers);

    if (key == Key::LetterR || key == Key::PageUp) {
        if (currentRotation < 23) {
            ++currentRotation;
        } else {
            currentRotation = 0;
        }
    } else if (key == Key::PageDown) {
        if (currentRotation > 0) {
            --currentRotation;
        } else {
            currentRotation = 23;
        }
    }
}

void ViewBuild::eventKeyReleased(const Key key, const Modifiers modifiers) {
    scene.eventKeyReleased(key, modifiers);
}

void ViewBuild::eventCharTyped(const uint32_t code) {
    scene.eventCharTyped(code);
}

void ViewBuild::onEnter() {
}

void ViewBuild::onExit() {
}

Scene* ViewBuild::getScene() {
    return &scene;
}
