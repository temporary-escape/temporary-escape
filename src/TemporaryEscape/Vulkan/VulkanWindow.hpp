#pragma once

#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"
#include "VEZ/AppBase.h"
#include "VulkanDevice.hpp"
#include "Window.hpp"

namespace Engine {
class VulkanWindow : public vez::AppBase, public VulkanDevice, public Window {
public:
    explicit VulkanWindow(const std::string& name, const Vector2i& size);
    virtual ~VulkanWindow() = default;

    virtual void initialize() = 0;
    virtual void cleanup() = 0;
    virtual void update(float timeElapsed) = 0;
    virtual void eventResized(const Vector2i& size) = 0;

    VkDevice getDevice() override;
    VezFramebuffer getFramebuffer() override;
    VezSwapchain getSwapchain() override;
    VkImage getColorAttachment() override;

protected:
    void Initialize() override;
    void Cleanup() override;
    void Draw() override;
    void OnKeyDown(int key, int mods) override;
    void OnKeyUp(int key, int mods) override;
    void OnMouseDown(int button, int x, int y) override;
    void OnMouseMove(int x, int y) override;
    void OnMouseUp(int button, int x, int y) override;
    void OnMouseScroll(float x, float y) override;
    void OnResize(int width, int height) override;
    void Update(float timeElapsed) override;

private:
    Vector2i mousePos;
};
} // namespace Engine
