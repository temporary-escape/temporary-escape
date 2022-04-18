#include "OpenGLWindow.hpp"
#include "../Utils/Log.hpp"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <iostream>
#include <stdexcept>

#define CMP "OpenGLWindow"

using namespace Engine;

static void errorCallback(const int error, const char* description) {
    Log::e(CMP, "{}", description);
}

OpenGLWindow::OpenGLWindow(const std::string& name, const Vector2i& size) : window(nullptr) {
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // Basic GLFW window stuff
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(size.x, size.y, name.c_str(), nullptr, nullptr);
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, &keyCallback);
    glfwSetCursorPosCallback(window, &mouseMovedCallback);
    glfwSetMouseButtonCallback(window, &mouseButtonCallback);
    glfwSetScrollCallback(window, &mouseScrollCallback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    glfwSwapInterval(1);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glEnable(GL_BLEND);
    glEnable(GL_RENDERBUFFER);
    glEnable(GL_FRAMEBUFFER);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT);
}

OpenGLWindow::~OpenGLWindow() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void OpenGLWindow::run() {
    while (!glfwWindowShouldClose(window)) {
        int width, height = 0;
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);

        render({width, height});

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void OpenGLWindow::mouseMovedCallback(GLFWwindow* window, const double x, const double y) {
    auto& self = *static_cast<OpenGLWindow*>(glfwGetWindowUserPointer(window));
    self.mousePos = {static_cast<int>(x), static_cast<int>(y)};
    self.eventMouseMoved(self.mousePos);
}

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

void OpenGLWindow::mouseButtonCallback(GLFWwindow* window, const int button, const int action, int mods) {
    (void)mods;

    auto& self = *static_cast<OpenGLWindow*>(glfwGetWindowUserPointer(window));

    const auto btn = toMouseButton(button);
    if (action == GLFW_PRESS) {
        self.eventMousePressed(self.mousePos, btn);
    } else if (action == GLFW_RELEASE) {
        self.eventMouseReleased(self.mousePos, btn);
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

void OpenGLWindow::keyCallback(GLFWwindow* window, const int key, const int scancode, const int action,
                               const int mods) {
    (void)scancode;
    (void)mods;

    auto& self = *static_cast<OpenGLWindow*>(glfwGetWindowUserPointer(window));

    Modifiers modifiers = 0;

    const auto k = toKey(key);
    if (action == GLFW_PRESS) {
        self.eventKeyPressed(k, modifiers);
    } else if (action == GLFW_RELEASE) {
        self.eventKeyReleased(k, modifiers);
    }
}

void OpenGLWindow::mouseScrollCallback(GLFWwindow* window, const double xscroll, const double yscroll) {
    auto& self = *static_cast<OpenGLWindow*>(glfwGetWindowUserPointer(window));

    self.eventMouseScroll(static_cast<int>(xscroll), static_cast<int>(yscroll));
}
