#pragma once

#include "../Vulkan/VulkanWindow.hpp"
#include "Game.hpp"

namespace Engine {
class ClientWindow : public VulkanWindow, public Game {
public:
    explicit ClientWindow(const Config& config);
    virtual ~ClientWindow();

    void update(float deltaTime) override;
    void render(const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventWindowResized(const Vector2i& size) override;

private:
    struct CameraUniformBuffer {
        Matrix4 model;
        Matrix4 view;
        Matrix4 projection;
    };

    float lastDeltaTime;
    VulkanBuffer vbo;
    VulkanBuffer ibo;
    VulkanBuffer ubo;
    VulkanPipeline shader;
};
} // namespace Engine
