#pragma once

#include "../Window.hpp"
#include "VulkanTypes.hpp"

struct GLFWwindow;

namespace Engine {
struct MonitorInfo {
    std::string name;
    bool primary;
};

ENGINE_API std::vector<MonitorInfo> listSystemMonitors();

class ENGINE_API VulkanWindow : public Window {
public:
    explicit VulkanWindow(const Config& config);
    virtual ~VulkanWindow();

    void run();
    Vector2i getFramebufferSize();
    void waitUntilValidFramebufferSize();
    void closeWindow();
    void setWindowMode(WindowMode value, const Vector2i& size, const std::string& monitorName);
    std::vector<Vector2i> getSupportedResolutionModes();

protected:
    virtual void onNextFrame() = 0;
    virtual void onExit() = 0;
    virtual void onFrameDraw(const Vector2i& viewport, float timeDelta) = 0;
    static std::vector<const char*> getRequiredExtensions();
    VkSurfaceKHR createSurface(VkInstance instance);

private:
    static void mouseMovedCallback(GLFWwindow* window, double x, double y);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void mouseScrollCallback(GLFWwindow* window, double xscroll, double yscroll);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void windowSizeCallback(GLFWwindow* window, int width, int height);
    static void charCallback(GLFWwindow* window, unsigned int codepoint);

    std::shared_ptr<GLFWwindow> window;
    Vector2i mousePos{};
    Vector2i currentWindowSize;
    WindowMode windowMode{WindowMode::Windowed};
};
} // namespace Engine
