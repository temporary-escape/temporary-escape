#include "VulkanWindow.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

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
    case GLFW_KEY_RIGHT_CONTROL: {
        return Key::RightControl;
    }
    case GLFW_KEY_DELETE: {
        return Key::Delete;
    }
    case GLFW_KEY_LEFT_SHIFT: {
        return Key::LeftShift;
    }
    case GLFW_KEY_RIGHT_SHIFT: {
        return Key::RightShift;
    }
    case GLFW_KEY_BACKSPACE: {
        return Key::Backspace;
    }
    case GLFW_KEY_ENTER: {
        return Key::Enter;
    }
    case GLFW_KEY_TAB: {
        return Key::Tab;
    }
    case GLFW_KEY_PAGE_UP: {
        return Key::PageUp;
    }
    case GLFW_KEY_PAGE_DOWN: {
        return Key::PageDown;
    }
    case GLFW_KEY_ESCAPE: {
        return Key::Escape;
    }
    default: {
        return Key::None;
    }
    }
}

static void errorCallback(const int error, const char* description) {
    (void)error;

    logger.error("{}", description);
}

static const char* name = "TemporaryEscape";

VulkanWindow::VulkanWindow(const Engine::Config& config) :
    currentWindowSize{config.graphics.windowSize}, mousePos{}, isFullScreen{config.graphics.fullscreen} {

    if (volkInitialize() != VK_SUCCESS) {
        EXCEPTION("Failed to initialize Vulkan");
    }

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        EXCEPTION("Failed to initialize GLFW");
    }

    if (!glfwVulkanSupported()) {
        EXCEPTION("Vulkan is not supported on this platform");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

    GLFWmonitor* monitor{nullptr};
    if (isFullScreen) {
        monitor = glfwGetPrimaryMonitor();
        const auto* mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    }

    auto windowPtr = glfwCreateWindow(currentWindowSize.x, currentWindowSize.y, name, monitor, nullptr);
    if (!windowPtr) {
        EXCEPTION("Failed to create GLFW window");
    }

    window = std::shared_ptr<GLFWwindow>(windowPtr, [](auto* w) { glfwDestroyWindow(w); });

    glfwSetWindowUserPointer(window.get(), this);
    glfwSetKeyCallback(window.get(), &keyCallback);
    glfwSetCursorPosCallback(window.get(), &mouseMovedCallback);
    glfwSetMouseButtonCallback(window.get(), &mouseButtonCallback);
    glfwSetScrollCallback(window.get(), &mouseScrollCallback);
    glfwSetWindowSizeCallback(window.get(), &windowSizeCallback);
    glfwSetCharCallback(window.get(), &charCallback);
}

VulkanWindow::~VulkanWindow() {
    // Terminate the GLFW library.
    glfwTerminate();
}

void VulkanWindow::run() {
    // Display the window.
    glfwShowWindow(window.get());

    auto timePoint = std::chrono::high_resolution_clock::now();

    // Message loop.
    while (!glfwWindowShouldClose(window.get())) {
        onNextFrame();

        // Check for window messages to process.
        glfwPollEvents();

        const auto windowSize = getFramebufferSize();
        if (windowSize != currentWindowSize) {
            // resizeDefaultFramebuffer(windowSize);
            currentWindowSize = windowSize;
            eventWindowResized(windowSize);
        }

        const auto now = std::chrono::high_resolution_clock::now();
        const auto timeDiff = std::chrono::duration_cast<std::chrono::microseconds>(now - timePoint);
        timePoint = now;

        onFrameDraw(windowSize, static_cast<float>(timeDiff.count()) / 1000000.0f);
    }

    onExit();
}

std::vector<const char*> VulkanWindow::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    return extensions;
}

VkSurfaceKHR VulkanWindow::createSurface(VkInstance instance) {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window.get(), nullptr, &surface) != VK_SUCCESS) {
        EXCEPTION("Failed to create window surface!");
    }
    return surface;
}

Vector2i VulkanWindow::getFramebufferSize() {
    int width, height;
    glfwGetFramebufferSize(window.get(), &width, &height);
    return {width, height};
}

void VulkanWindow::waitUntilValidFramebufferSize() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window.get(), &width, &height);

    auto eventTriggered = false;

    while (width == 0 || height == 0) {
        if (!eventTriggered) {
            eventTriggered = true;
            eventWindowBlur();
        }

        glfwGetFramebufferSize(window.get(), &width, &height);
        glfwWaitEvents();
    }

    if (eventTriggered) {
        eventWindowFocus();
    }
}

void VulkanWindow::closeWindow() {
    glfwSetWindowShouldClose(window.get(), GL_TRUE);
}

void VulkanWindow::mouseMovedCallback(GLFWwindow* window, double x, double y) {
    auto& self = *static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
    self.mousePos = {static_cast<int>(x), static_cast<int>(y)};
    self.eventMouseMoved(self.mousePos);
}

void VulkanWindow::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;

    auto& self = *static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));

    const auto btn = toMouseButton(button);
    if (action == GLFW_PRESS) {
        self.eventMousePressed(self.mousePos, btn);
    } else if (action == GLFW_RELEASE) {
        self.eventMouseReleased(self.mousePos, btn);
    }
}

void VulkanWindow::mouseScrollCallback(GLFWwindow* window, double xscroll, double yscroll) {
    auto& self = *static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));

    self.eventMouseScroll(static_cast<int>(xscroll), static_cast<int>(yscroll));
}

void VulkanWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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

void VulkanWindow::windowSizeCallback(GLFWwindow* window, int width, int height) {
    // auto& self = *static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));

    const Vector2i size = {width, height};
    logger.debug("windowSizeCallback new size: {}", size);
}

void VulkanWindow::charCallback(GLFWwindow* window, unsigned int codepoint) {
    auto& self = *static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
    self.eventCharTyped(codepoint);
}

std::vector<Vector2i> VulkanWindow::getSupportedResolutionModes() {
    auto* primary = glfwGetPrimaryMonitor();

    std::vector<Vector2i> result;

    int count;
    auto* modes = glfwGetVideoModes(primary, &count);
    result.reserve(count);

    for (auto i = 0; i < count; i++) {
        const auto contains = std::any_of(result.begin(), result.end(), [&](const Vector2i& v) {
            return v.x == modes[i].width && v.y == modes[i].height;
        });
        if (!contains) {
            result.emplace_back(modes[i].width, modes[i].height);
        }
    }

    return result;
}

void VulkanWindow::setWindowFullScreen(const bool value) {
    if (value) {
        logger.info("Setting window to fullscreen");
        if (!isFullScreen) {
            isFullScreen = true;
            auto* primary = glfwGetPrimaryMonitor();
            const auto* mode = glfwGetVideoMode(primary);

            glfwSetWindowMonitor(
                window.get(), primary, 0, 0, currentWindowSize.x, currentWindowSize.y, mode->refreshRate);
        }
    } else {
        logger.info("Setting window to windowed");
        if (isFullScreen) {
            isFullScreen = false;
            auto* primary = glfwGetPrimaryMonitor();
            const auto* mode = glfwGetVideoMode(primary);
            const auto pos = Vector2i{mode->width, mode->height} / 2 - currentWindowSize / 2;

            glfwSetWindowMonitor(window.get(), nullptr, pos.x, pos.y, currentWindowSize.x, currentWindowSize.y, 0);
        }
    }
}

void VulkanWindow::setWindowResolution(const Vector2i& size) {
    currentWindowSize = size;
    glfwSetWindowSize(window.get(), currentWindowSize.x, currentWindowSize.y);
}
