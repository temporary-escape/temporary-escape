#pragma once

#include "../Assets/AssetsManager.hpp"
#include "../Scene/Scene.hpp"
#include "../Server/Schemas.hpp"
#include "View.hpp"

namespace Engine {
class ENGINE_API ViewBuild : public View {
public:
    explicit ViewBuild(const Config& config, VulkanRenderer& vulkan, GuiManager& guiManager, AudioContext& audio,
                       AssetsManager& assetsManager, VoxelShapeCache& voxelShapeCache, FontFamily& font);
    ~ViewBuild() = default;

    void update(float deltaTime, const Vector2i& viewport) override;
    void renderCanvas(Canvas& canvas, const Vector2i& viewport) override;
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
    static constexpr size_t maxHistoryItems = 100;

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
