#pragma once

#include "../Vulkan/VulkanWindow.hpp"

namespace Engine {
class ClientWindow : public VulkanWindow {
public:
    explicit ClientWindow(const std::string& name, const Vector2i& size);
    virtual ~ClientWindow();

    void initialize() override;
    void cleanup() override;
    void update(float timeElapsed) override;
    void eventResized(const Vector2i& size) override;
    void render(const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;

private:
    struct CameraUniformBuffer {
        Matrix4 model;
        Matrix4 view;
        Matrix4 projection;
    };

    VulkanBuffer vbo;
    VulkanBuffer ibo;
    VulkanBuffer ubo;
    VulkanPipeline shader;
};
} // namespace Engine
