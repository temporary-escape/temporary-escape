#pragma once

#include "../window.hpp"
#include "vulkan_types.hpp"

struct GLFWwindow;

namespace Engine {
class ENGINE_API VulkanWindow : public Window {
public:
    explicit VulkanWindow(const Config& config);
    virtual ~VulkanWindow();

    void run();
    Vector2i getFramebufferSize();
    void waitUntilValidFramebufferSize();

protected:
    virtual void onNextFrame() = 0;
    virtual void onExit() = 0;
    virtual void onFrameDraw(const Vector2i& viewport, float timeDelta) = 0;
    std::vector<const char*> getRequiredExtensions();
    VkSurfaceKHR createSurface(VkInstance instance);

private:
    static void mouseMovedCallback(GLFWwindow* window, double x, double y);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void mouseScrollCallback(GLFWwindow* window, double xscroll, double yscroll);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void windowSizeCallback(GLFWwindow* window, int width, int height);
    static void charCallback(GLFWwindow* window, unsigned int codepoint);

    std::shared_ptr<GLFWwindow> window;
    Vector2i mousePos;
    Vector2i currentWindowSize;
};
} // namespace Engine
