#pragma once

#include "../assets/registry.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../gui/gui_block_action_bar.hpp"
#include "../gui/gui_block_selector.hpp"
#include "../gui/gui_side_menu.hpp"
#include "../scene/scene.hpp"
#include "view.hpp"

namespace Engine {
class ENGINE_API ViewBuild : public View {
public:
    struct Gui {
        explicit Gui(const Config& config, Registry& registry);

        GuiBlockActionBar blockActionBar;
        GuiBlockSelector blockSelector;
        GuiSideMenu blockSideMenu;
    };

    explicit ViewBuild(const Config& config, Renderer& renderer, Registry& registry, Gui& gui);
    ~ViewBuild() = default;

    void update(float deltaTime) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;
    void onEnter() override;
    void onExit() override;
    Scene& getScene() override;

private:
    void createScene();
    void createGridLines();
    void createEntityShip();
    void createHelpers();
    EntityPtr createHelperBox(const Color4& color, float width);
    void addBlock();

    const Config& config;
    Registry& registry;
    Gui& gui;

    Vector2 raycastScreenPos;
    std::optional<Grid::RayCastResult> raycastResult;

    Skybox skybox;
    Scene scene;
    EntityPtr entityShip;
    EntityPtr entityHelperAdd;
    EntityPtr entityHelperRemove;
};
} // namespace Engine
