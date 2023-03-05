#pragma once

#include "../assets/registry.hpp"
#include "../assets/voxel_shape_cache.hpp"
#include "../config.hpp"
#include "../database/rocks_db.hpp"
#include "../font/font_family.hpp"
#include "../future.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../graphics/renderer.hpp"
#include "../graphics/skybox_generator.hpp"
#include "../server/server.hpp"
#include "client.hpp"
#include "stats.hpp"
#include "view_build.hpp"
#include "view_galaxy.hpp"
#include "view_space.hpp"
#include "view_system.hpp"

namespace Engine {
class ENGINE_API TransactionalDatabase;

class ENGINE_API Editor : public UserInput {
public:
    explicit Editor(const Config& config, Renderer& renderer, Canvas& canvas, Nuklear& nuklear, Registry& registry,
                    FontFamily& font);
    virtual ~Editor();

    void update(float deltaTime);
    void render(const Vector2i& viewport);

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

private:
    const Config& config;
    Renderer& renderer;
    Canvas& canvas;
    Nuklear& nuklear;
    Registry& registry;
    FontFamily& font;
    Gui gui;
    ViewBuild view;
};
} // namespace Engine
