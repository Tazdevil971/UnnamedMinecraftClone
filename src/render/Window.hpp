#pragma once

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

namespace render {

class Window {
   public:
    struct InputState {
        bool forward;
        bool backward;
        bool left;
        bool right;
        bool jump;
        bool crouch;
        bool place;
        bool destroy;
        glm::vec2 cursorPos;
        float deltaTime;
    };

    Window(std::string name);

    void mainLoop();
    void cleanup();

    float getTime();

   protected:
    virtual void onFrame(InputState &input) {}
    virtual void onResize(int width, int height) {}

    GLFWwindow *getWindow() const { return window; }

   private:
    static void glfwOnResizeCallback(GLFWwindow *window, int width, int height);

    GLFWwindow *window{nullptr};

    bool captureMouse{false};
};

}  // namespace render