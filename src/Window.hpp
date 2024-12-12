#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

#include "render/BufferManager.hpp"
#include "render/Context.hpp"
#include "render/Renderer.hpp"
#include "render/TextureManager.hpp"

class Window {
   public:
    Window();

    void mainLoop();
    void cleanup();

    void onResize(int width, int height);
   private:

    float getTime();

    GLFWwindow *window;
    bool windowResized{false};

    std::shared_ptr<render::Context> ctx;
    std::shared_ptr<render::BufferManager> bufferMgr;
    std::shared_ptr<render::TextureManager> textureMgr;
    std::shared_ptr<render::Renderer> renderer;

    render::SimpleModel model;
};