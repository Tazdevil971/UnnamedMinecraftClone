#include "Window.hpp"

#include <chrono>

using namespace render;

void Window::glfwOnResizeCallback(GLFWwindow *window, int width, int height) {
    auto app = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    app->onResize(width, height);
}

Window::Window(std::string name) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window =
        glfwCreateWindow(1280, 720, name.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, glfwOnResizeCallback);
}

void Window::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        onFrame();
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