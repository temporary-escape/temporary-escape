#pragma once

#include "../assets/assets_manager.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../gui/gui_block_info.hpp"
#include "../gui/gui_block_selector.hpp"
#include "../gui/gui_file_browser.hpp"
#include "../scene/scene.hpp"
#include "../server/schemas.hpp"
#include "view.hpp"

namespace Engine {
class ENGINE_API ViewBuild : public View {
public:
    explicit ViewBuild(const Config& config, VulkanRenderer& vulkan, AudioContext& audio, AssetsManager& assetsManager,
                       VoxelShapeCache& voxelShapeCache);
    ~ViewBuild() = default;

    void update(float deltaTime) override;
    void renderCanvas(Canvas& canvas, const Vector2i& viewport) override;
    void renderNuklear(Nuklear& nuklear, const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;
    void onEnter() override;
    void onExit() override;
    Scene* getScene() override;

private:
    static constexpr size_t maxHistoryItems = 10;

    struct Action {
        bool added{false};
        BlockPtr block{nullptr};
        VoxelShape::Type shape{VoxelShape::Type::Cube};
        Vector3i pos;
        size_t rotation{0};
        size_t color{0};
    };

    void createScene();
    void createGridLines();
    void createEntityShip();
    void createHelpers();
    Entity createHelperBox(const Color4& color, float width);
    void addBlock();
    void removeBlock();
    void updateSelectedBlock();
    void doUndo();
    void doRedo();
    void doSave();
    void doLoad();
    void resetHistory();

    const Config& config;
    VulkanRenderer& vulkan;
    AssetsManager& assetsManager;
    AudioSource uiAudioSource;
    GuiBlockSelector guiBlockSelector;
    GuiBlockInfo guiBlockInfo;
    GuiFileBrowser guiFileBrowser;
    // GuiBlockActionBar guiBlockActionBar;
    // GuiBlockSelector guiBlockSelector;
    // GuiSideMenu guiBlockSideMenu;

    Vector2 raycastScreenPos;
    std::optional<Grid::RayCastResult> raycastResult;

    Scene scene;
    Entity entityShip;
    Entity entityHelperAdd;
    Entity entityHelperRemove;
    size_t currentRotation{0};

    struct {
        SoundPtr build;
        SoundPtr destroy;
    } sound;

    struct {
        BlockPtr block{nullptr};
        VoxelShape::Type shape{VoxelShape::Cube};
        size_t color{0};
        size_t rotation{0};
    } selected;

    std::deque<Action> history{};
    int64_t historyPos{0};
};
} // namespace Engine
