#include "PlayerController.hpp"

#include <glm/gtc/matrix_transform.hpp>

using namespace render;

PlayerController::PlayerController() : Window("UnnamedMinecraftClone") {
    try {
        ctx = Context::create(getWindow());
        bufferMgr = BufferManager::create(ctx);
        textureMgr = TextureManager::create(ctx, bufferMgr, 10);
        swapchain = Swapchain::create(ctx, bufferMgr);
        renderer = Renderer::create(ctx, textureMgr, swapchain);

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

void PlayerController::cleanup() {
    // Wait for the device to finish rendering before cleaning up!
    ctx->waitDeviceIdle();

    textureMgr->deallocateSimpleTexture(model.texture);
    bufferMgr->deallocateSimpleMesh(model.mesh);

    if (renderer) renderer->cleanup();

    if (swapchain) swapchain->cleanup();

    if (textureMgr) textureMgr->cleanup();

    if (bufferMgr) bufferMgr->cleanup();

    if (ctx) ctx->cleanup();

    Window::cleanup();
}

void PlayerController::onFrame() {
    Camera camera{
        glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, -1.0f)),
        45.0f, 0.1f, 10.0f};

    model.modelMatrix =
        glm::rotate(glm::mat4(1.0f), getTime() * glm::radians(90.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));

    renderer->render(camera, {model}, windowResized);
    windowResized = false;
}

void PlayerController::onResize(int width, int height) { windowResized = true; }