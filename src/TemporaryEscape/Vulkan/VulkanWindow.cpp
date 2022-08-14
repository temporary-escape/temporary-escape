#include "VulkanWindow.hpp"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define CMP "VulkanWindow"

using namespace Engine;

static MouseButton toMouseButton(int button) {
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT: {
        return MouseButton::Left;
    }
    case GLFW_MOUSE_BUTTON_MIDDLE: {
        return MouseButton::Middle;
    }
    case GLFW_MOUSE_BUTTON_RIGHT: {
        return MouseButton::Right;
    }
    default: {
        return MouseButton::None;
    }
    }
}

static Key toKey(int key) {
    switch (key) {
    case GLFW_KEY_A: {
        return Key::LetterA;
    }
    case GLFW_KEY_B: {
        return Key::LetterB;
    }
    case GLFW_KEY_C: {
        return Key::LetterC;
    }
    case GLFW_KEY_D: {
        return Key::LetterD;
    }
    case GLFW_KEY_E: {
        return Key::LetterE;
    }
    case GLFW_KEY_F: {
        return Key::LetterF;
    }
    case GLFW_KEY_G: {
        return Key::LetterG;
    }
    case GLFW_KEY_H: {
        return Key::LetterH;
    }
    case GLFW_KEY_I: {
        return Key::LetterI;
    }
    case GLFW_KEY_J: {
        return Key::LetterJ;
    }
    case GLFW_KEY_K: {
        return Key::LetterK;
    }
    case GLFW_KEY_L: {
        return Key::LetterL;
    }
    case GLFW_KEY_M: {
        return Key::LetterM;
    }
    case GLFW_KEY_N: {
        return Key::LetterN;
    }
    case GLFW_KEY_O: {
        return Key::LetterO;
    }
    case GLFW_KEY_P: {
        return Key::LetterP;
    }
    case GLFW_KEY_Q: {
        return Key::LetterQ;
    }
    case GLFW_KEY_R: {
        return Key::LetterR;
    }
    case GLFW_KEY_S: {
        return Key::LetterS;
    }
    case GLFW_KEY_T: {
        return Key::LetterT;
    }
    case GLFW_KEY_V: {
        return Key::LetterV;
    }
    case GLFW_KEY_W: {
        return Key::LetterW;
    }
    case GLFW_KEY_U: {
        return Key::LetterU;
    }
    case GLFW_KEY_X: {
        return Key::LetterX;
    }
    case GLFW_KEY_Y: {
        return Key::LetterY;
    }
    case GLFW_KEY_Z: {
        return Key::LetterZ;
    }
    case GLFW_KEY_SPACE: {
        return Key::SpaceBar;
    }
    case GLFW_KEY_LEFT_CONTROL: {
        return Key::LeftControl;
    }
    case GLFW_KEY_DELETE: {
        return Key::Delete;
    }
    case GLFW_KEY_LEFT_SHIFT: {
        return Key::LeftShift;
    }
    default: {
        return Key::None;
    }
    }
}

static void setWindowCenter(GLFWwindow* window) {
    // Get window position and size
    int posX, posY;
    glfwGetWindowPos(window, &posX, &posY);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Halve the window size and use it to adjust the window position to the center of the window
    width >>= 1;
    height >>= 1;

    posX += width;
    posY += height;

    // Get the list of monitors
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    if (monitors == nullptr)
        return;

    // Figure out which monitor the window is in
    GLFWmonitor* owner = NULL;
    int owner_x, owner_y, owner_width, owner_height;

    for (int i = 0; i < count; i++) {
        // Get the monitor position
        int monitor_x, monitor_y;
        glfwGetMonitorPos(monitors[i], &monitor_x, &monitor_y);

        // Get the monitor size from its video mode
        int monitor_width, monitor_height;
        GLFWvidmode* monitor_vidmode = (GLFWvidmode*)glfwGetVideoMode(monitors[i]);

        if (monitor_vidmode == NULL)
            continue;

        monitor_width = monitor_vidmode->width;
        monitor_height = monitor_vidmode->height;

        // Set the owner to this monitor if the center of the window is within its bounding box
        if ((posX > monitor_x && posX < (monitor_x + monitor_width)) &&
            (posY > monitor_y && posY < (monitor_y + monitor_height))) {
            owner = monitors[i];
            owner_x = monitor_x;
            owner_y = monitor_y;
            owner_width = monitor_width;
            owner_height = monitor_height;
        }
    }

    // Set the window position to the center of the owner monitor
    if (owner != NULL)
        glfwSetWindowPos(window, owner_x + (owner_width >> 1) - width, owner_y + (owner_height >> 1) - height);
}

static void errorCallback(const int error, const char* description) {
    Log::e(CMP, "{}", description);
}

VulkanWindow::VulkanWindow(const std::string& name, const Vector2i& size, bool enableValidationLayers) :
    currentWindowSize(size) {
    const auto standardValidationFound = getValidationLayerSupported();

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        EXCEPTION("Failed to initialize GLFW");
    }

    if (!glfwVulkanSupported()) {
        EXCEPTION("Vulkan is not supported on this platform");
    }

    // Initialize a Vulkan instance with the validation layers enabled and extensions required by glfw.
    uint32_t instanceExtensionCount = 0U;
    auto instanceExtensionsStr = glfwGetRequiredInstanceExtensions(&instanceExtensionCount);

    std::vector<const char*> instanceExtensions;
    for (auto i = 0; i < instanceExtensionCount; i++) {
        instanceExtensions.push_back(instanceExtensionsStr[i]);
    }

    std::vector<const char*> instanceLayers;
    if (enableValidationLayers && standardValidationFound) {
        instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
    }

    if (enableValidationLayers && !standardValidationFound) {
        Log::e(CMP, "Vulkan validation layers requested but not found");
    }

    initInstance(name, instanceLayers, instanceExtensions);

    // Initialize a window using GLFW and hint no graphics API should be used on the backend.
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
    window = glfwCreateWindow(size.x, size.y, name.c_str(), nullptr, nullptr);
    if (!window) {
        EXCEPTION("Failed to create GLFW window");
    }

    setWindowCenter(window);

    initSurface(window);

    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, &keyCallback);
    glfwSetCursorPosCallback(window, &mouseMovedCallback);
    glfwSetMouseButtonCallback(window, &mouseButtonCallback);
    glfwSetScrollCallback(window, &mouseScrollCallback);
    glfwSetWindowSizeCallback(window, windowSizeCallback);

    // Display the window.
    glfwShowWindow(window);
}

VulkanWindow::~VulkanWindow() {
    reset();

    // Destroy the GLFW window.
    glfwDestroyWindow(window);

    // Terminate the GLFW library.
    glfwTerminate();
}

Vector2i VulkanWindow::getWindowSize() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return {width, height};
}

void VulkanWindow::run() {
    // Track time elapsed from one Update call to the next.
    double lastTime = glfwGetTime();

    // Message loop.
    while (!glfwWindowShouldClose(window)) {
        // Check for window messages to process.
        glfwPollEvents();

        const auto windowSize = getWindowSize();
        if (windowSize != currentWindowSize) {
            resizeDefaultFramebuffer(windowSize);
            currentWindowSize = windowSize;
            eventWindowResized(windowSize);
        }

        double curTime = glfwGetTime();
        render(windowSize, static_cast<float>(curTime - lastTime));
        lastTime = curTime;
    }

    deviceWaitIdle();
}

void VulkanWindow::mouseMovedCallback(GLFWwindow* window, const double x, const double y) {
    auto& self = *static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
    self.mousePos = {static_cast<int>(x), static_cast<int>(y)};
    self.eventMouseMoved(self.mousePos);
}

void VulkanWindow::mouseButtonCallback(GLFWwindow* window, const int button, const int action, int mods) {
    (void)mods;

    auto& self = *static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));

    const auto btn = toMouseButton(button);
    if (action == GLFW_PRESS) {
        self.eventMousePressed(self.mousePos, btn);
    } else if (action == GLFW_RELEASE) {
        self.eventMouseReleased(self.mousePos, btn);
    }
}

void VulkanWindow::keyCallback(GLFWwindow* window, const int key, const int scancode, const int action,
                               const int mods) {
    (void)scancode;
    (void)mods;

    auto& self = *static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));

    Modifiers modifiers = 0;

    const auto k = toKey(key);
    if (action == GLFW_PRESS) {
        self.eventKeyPressed(k, modifiers);
    } else if (action == GLFW_RELEASE) {
        self.eventKeyReleased(k, modifiers);
    }
}

void VulkanWindow::mouseScrollCallback(GLFWwindow* window, const double xscroll, const double yscroll) {
    auto& self = *static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));

    self.eventMouseScroll(static_cast<int>(xscroll), static_cast<int>(yscroll));
}

void VulkanWindow::windowSizeCallback(GLFWwindow* window, int width, int height) {
    auto& self = *static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));

    const Vector2i size = {width, height};

    Log::d(CMP, "windowSizeCallback new size: {}", size);
}
