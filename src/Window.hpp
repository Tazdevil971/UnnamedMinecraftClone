#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class Window {
public:
    Window();

    void mainLoop();
    void cleanup();

private:
    void initWindow(int width, int height, const char *title);
    void initVulkan();
    
    void onResize(int width, int height);

    std::vector<const char*> getRequiredVkExtensions();

    static void glfwOnResizeCallback(GLFWwindow *window, int width, int height);

    GLFWwindow *window = nullptr;
    VkInstance instance;
    VkSurfaceKHR surface;
};