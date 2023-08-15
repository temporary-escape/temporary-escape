#pragma once

#include "../assets/assets_manager.hpp"
#include "../assets/voxel_shape_cache.hpp"
#include "../config.hpp"
#include "../database/database_rocksdb.hpp"
#include "../font/font_family.hpp"
#include "../future.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../graphics/renderer.hpp"
#include "../graphics/renderer_planet_surface.hpp"
#include "../graphics/renderer_skybox.hpp"
#include "../server/server.hpp"
#include "client.hpp"
#include "stats.hpp"
#include "view_build.hpp"
#include "view_galaxy.hpp"
#include "view_space.hpp"
#include "view_system.hpp"

namespace Engine {
class ENGINE_API Database;

class ENGINE_API Game : public UserInput {
public:
    explicit Game(const Config& config, VulkanRenderer& vulkan, RendererSkybox& rendererSkybox,
                  RendererPlanetSurface& rendererPlanetSurface, AssetsManager& assetsManager,
                  VoxelShapeCache& voxelShapeCache, FontFamily& font, Client& client);
    virtual ~Game();

    bool isReady() const;
    void update(float deltaTime);
    void render(VulkanCommandBuffer& vkb, Renderer& renderer, const Vector2i& viewport);
    void renderCanvas(Canvas& canvas, Nuklear& nuklear, const Vector2i& viewport);

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

    void switchToGalaxyMap();
    void switchToSystemMap();
    void switchToSystemMap(const std::string& galaxyId, const std::string& systemId);

private:
    const Config& config;
    RendererSkybox& rendererSkybox;
    RendererPlanetSurface& rendererPlanetSurface;
    AssetsManager& assetsManager;
    FontFamily& font;
    Client& client;
    uint32_t selectedEntityId{0xFFFFFFFF};

    std::unique_ptr<ViewBuild> viewBuild;
    std::unique_ptr<ViewSpace> viewSpace;
    std::unique_ptr<ViewGalaxy> viewGalaxy;
    std::unique_ptr<ViewSystem> viewSystem;
    View* view{nullptr};
};
} // namespace Engine
