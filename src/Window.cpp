#include "Window.hpp"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace render;

void glfwOnResizeCallback(GLFWwindow *window, int width, int height) {
    auto app = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    app->onResize(width, height);
}

Window::Window() {
    try {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(1280, 720, "UnnamedMinecraftClone", nullptr,
                                  nullptr);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, glfwOnResizeCallback);

        ctx = Context::create(window);
        bufferMgr = BufferManager::create(ctx);
        textureMgr = TextureManager::create(ctx, bufferMgr, 10);
        renderer = Renderer::create(ctx, textureMgr, bufferMgr);

        model.texture = textureMgr->createSimpleTexture(
            "assets/test_image.jpg", VK_FORMAT_R8G8B8A8_SRGB);

        model.mesh = bufferMgr->allocateSimpleMesh(
            {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4},
            {{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
             {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
             {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
             {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

             {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
             {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
             {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
             {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}});

    } catch (...) {
        cleanup();
        throw;
    }
}

void Window::cleanup() {
    textureMgr->deallocateSimpleTexture(model.texture);
    bufferMgr->deallocateSimpleMesh(model.mesh);

    if (renderer) renderer->cleanup();

    if (textureMgr) textureMgr->cleanup();

    if (bufferMgr) bufferMgr->cleanup();

    if (ctx) ctx->cleanup();

    if (window) glfwDestroyWindow(window);

    glfwTerminate();
}

void Window::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        Camera camera{glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                                  glm::vec3(0.0f, 0.0f, 0.0f),
                                  glm::vec3(0.0f, 0.0f, -1.0f)),
                      45.0f, 0.1f, 10.0f};

        model.modelMatrix =
            glm::rotate(glm::mat4(1.0f), getTime() * glm::radians(90.0f),
                        glm::vec3(0.0f, 0.0f, 1.0f));

        renderer->render(camera, {model}, windowResized);
        windowResized = false;
    }

    vkDeviceWaitIdle(ctx->getDevice());
}

void Window::onResize(int width, int height) { windowResized = true; }

float Window::getTime() {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float, std::chrono::seconds::period>(
               currentTime - startTime)
        .count();
}