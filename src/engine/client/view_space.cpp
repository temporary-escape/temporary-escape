#include "view_space.hpp"
#include "../graphics/renderer.hpp"
#include "client.hpp"

#define CMP "ViewSpace"

using namespace Engine;

ViewSpace::ViewSpace(const Config& config, VulkanDevice& vulkan, Registry& registry, Canvas& canvas, FontFamily& font,
                     Nuklear& nuklear, Skybox& skybox, Client& client) :
    config{config},
    vulkan{vulkan},
    registry{registry},
    canvas{canvas},
    font{font},
    nuklear{nuklear},
    skyboxSystem{skybox},
    client{client} {
}

void ViewSpace::update(const float deltaTime) {
}

void ViewSpace::render(const Vector2i& viewport, Renderer& renderer) {
    auto scene = client.getScene();
    if (scene) {
        renderer.render(viewport, *scene, skyboxSystem);
    }
}

void ViewSpace::renderCanvas(const Vector2i& viewport) {
}

void ViewSpace::eventUserInput(const UserInput::Event& event) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventUserInput(event);
    }
}
