#include "Window.hpp"

#include <chrono>
#include <iostream>

using namespace render;

void Window::glfwOnResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->onResize(width, height);
}

Window::Window(std::string name) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(1280, 720, name.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, glfwOnResizeCallback);

    // Set sticky keys
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    captureMouse = true;
}

void Window::mainLoop() {
    InputState input;
    input.cursorPos.x = 0.0f;
    input.cursorPos.y = 0.0f;

    double xLast, yLast;
    glfwGetCursorPos(window, &xLast, &yLast);

    float timeStart = getTime();
    float timeLast = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (captureMouse) {
            input.forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
            input.backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
            input.left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
            input.right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
            input.jump = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
            input.crouch =
                glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
            input.place = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) ==
                          GLFW_PRESS;
            input.destroy = glfwGetMouseButton(
                                window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

            double x, y;
            glfwGetCursorPos(window, &x, &y);

            input.cursorPos.x += xLast - x;
            input.cursorPos.y += yLast - y;

            xLast = x;
            yLast = y;

        } else {
            input.forward = false;
            input.backward = false;
            input.left = false;
            input.right = false;
            input.jump = false;
            input.crouch = false;
            input.place = false;
            input.deltaTime = false;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS &&
            !captureMouse) {
            glfwGetCursorPos(window, &xLast, &yLast);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            captureMouse = true;
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && captureMouse) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            captureMouse = false;
        }

        input.time = getTime() - timeStart;
        input.deltaTime = input.time - timeLast;
        timeLast = input.time;

        onFrame(input);
    }
}

void Window::cleanup() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

float Window::getTime() {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float, std::chrono::seconds::period>(
               currentTime - startTime)
        .count();
}