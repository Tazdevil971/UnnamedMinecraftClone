#pragma once

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace render {

class Window {
public:
    Window(std::string name);

    void mainLoop();
    void cleanup();

    float getTime();

protected:
    virtual void onFrame() {}
    virtual void onResize(int width, int height) {}

    GLFWwindow *getWindow() const {
        return window;
    }

private:
    static void glfwOnResizeCallback(GLFWwindow *window, int width, int height);

    GLFWwindow *window{nullptr};
};

}