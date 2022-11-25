#pragma once

#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"
#include "vulkan_device.hpp"
#include "window.hpp"

struct GLFWwindow;

namespace Engine {
class VulkanWindow : public VulkanDevice, public Window {
public:
    explicit VulkanWindow(const Config& config, const std::string& name, const Vector2i& size,
                          bool enableValidationLayers = true);
    virtual ~VulkanWindow();

    void run();
    Vector2i getWindowSize() override;

private:
    static void mouseMovedCallback(GLFWwindow* window, double x, double y);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void mouseScrollCallback(GLFWwindow* window, double xscroll, double yscroll);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void windowSizeCallback(GLFWwindow* window, int width, int height);
    static void charCallback(GLFWwindow* window, unsigned int codepoint);

    GLFWwindow* window{nullptr};
    Vector2i mousePos;
    Vector2i currentWindowSize;
};
} // namespace Engine
