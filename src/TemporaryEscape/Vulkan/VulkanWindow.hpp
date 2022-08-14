#pragma once

#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"
#include "VulkanDevice.hpp"
#include "Window.hpp"

struct GLFWwindow;

namespace Engine {
class VulkanWindow : public VulkanDevice, public Window {
public:
    explicit VulkanWindow(const std::string& name, const Vector2i& size, bool enableValidationLayers = true);
    virtual ~VulkanWindow();

    void run();
    Vector2i getWindowSize() override;

private:
    static void mouseMovedCallback(GLFWwindow* window, double x, double y);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void mouseScrollCallback(GLFWwindow* window, double xscroll, double yscroll);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void windowSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* window{nullptr};
    Vector2i mousePos;
    Vector2i currentWindowSize;
};
} // namespace Engine
