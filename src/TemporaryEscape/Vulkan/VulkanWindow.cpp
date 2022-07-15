#include "VulkanWindow.hpp"

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
    default: {
        return Key::None;
    }
    }
}

VulkanWindow::VulkanWindow(const std::string& name, const Vector2i& size) : AppBase(name.c_str(), size.x, size.y) {
}

void VulkanWindow::Initialize() {
    vezDeviceWaitIdle(AppBase::GetDevice());
    vezSwapchainSetVSync(AppBase::GetSwapchain(), true);

    initialize();
}

void VulkanWindow::Cleanup() {
    cleanup();
    VulkanDevice::reset();
}

void VulkanWindow::Draw() {
    render({0, 0});
}

void VulkanWindow::OnKeyDown(int key, int mods) {
    Modifiers modifiers = 0;
    eventKeyPressed(toKey(key), modifiers);
}

void VulkanWindow::OnKeyUp(int key, int mods) {
    Modifiers modifiers = 0;
    eventKeyReleased(toKey(key), modifiers);
}

void VulkanWindow::OnMouseDown(int button, int x, int y) {
    const auto btn = toMouseButton(button);
    eventMousePressed(mousePos, btn);
}

void VulkanWindow::OnMouseMove(int x, int y) {
    mousePos = {x, y};
}

void VulkanWindow::OnMouseUp(int button, int x, int y) {
    const auto btn = toMouseButton(button);
    eventMouseReleased(mousePos, btn);
}

void VulkanWindow::OnMouseScroll(float x, float y) {
    eventMouseScroll(static_cast<int>(x), static_cast<int>(y));
}

void VulkanWindow::OnResize(int width, int height) {
    eventResized({width, height});
}

void VulkanWindow::Update(float timeElapsed) {
    update(timeElapsed);
}

VkDevice VulkanWindow::getDevice() {
    return AppBase::GetDevice();
}

VezSwapchain VulkanWindow::getSwapchain() {
    return AppBase::GetSwapchain();
}

VkImage VulkanWindow::getColorAttachment() {
    return AppBase::GetColorAttachment();
}

VezFramebuffer VulkanWindow::getFramebuffer() {
    return AppBase::GetFramebuffer();
}
