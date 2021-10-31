#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Config.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Gui/GuiContext.hpp"
#include "../Network/NetworkClient.hpp"
#include "../Scene/Camera.hpp"
#include "../Scene/Grid.hpp"
#include "../Scene/Scene.hpp"
#include "Store.hpp"
#include "View.hpp"
#include "Widgets.hpp"

namespace Scissio {
class SCISSIO_API ViewBuild : public View, public Store::Listener {
public:
    explicit ViewBuild(const Config& config, Network::Client& client, Store& store, AssetManager& assetManager);
    virtual ~ViewBuild() = default;

    void update(const Vector2i& viewport) override;
    void render(const Vector2i& viewport, Renderer& renderer) override;
    void renderCanvas(const Vector2i& viewport, Canvas2D& canvas, GuiContext& gui) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventMouseScroll(int xscroll, int yscroll) override;

private:
    enum class Mode {
        None = 0,
        Select,
        Build,
    };

    struct ActionPlaceBlock {
        Grid::BlockRef ref;
        Vector3 pos;
        int rot{0};
    };

    struct ActionRemoveBlock {
        Vector3 pos;
    };

    using Action = std::variant<ActionPlaceBlock, ActionRemoveBlock>;

    void calculateRayCast();
    void actionPlaceBlock();
    void actionRemoveBlock();
    void actionUndo();
    void actionRedo();
    Action action(const Action& action);
    void saveAction(Action action);

    const Config& config;
    Network::Client& client;
    AssetManager& assetManager;

    Scene scene;
    Camera camera;
    Vector2i viewport{};
    EntityPtr ship;
    EntityPtr preview;
    EntityPtr highlight;

    Mode mode;

    bool cameraMove[6];
    Vector2 cameraRotation;
    bool cameraRotate;
    Vector2 mousePosOld{};
    Vector2 mousePos{};

    bool loading;
    std::vector<Widgets::SidebarItem> sidebar;
    Widgets::BlockSelectorData blockSelector;
    std::optional<BlockDto> blockSelectorChoice;
    std::optional<Grid::RayCastResult> rayCastResult{};
    std::optional<std::reference_wrapper<Grid::BlockNode>> selectedBlock{};
    std::optional<Vector3> placePosition{};
    int placeRotation;

    std::list<Action> actionsUndo;
    std::list<Action> actionsRedo;
};
} // namespace Scissio
