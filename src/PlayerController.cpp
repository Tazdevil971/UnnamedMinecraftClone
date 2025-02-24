#include "PlayerController.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <math.h>

using namespace render;
using namespace world;

PlayerController::PlayerController() : Window("UnnamedMinecraftClone") {
    try {
        ctx = Context::create(getWindow());
        bufferMgr = BufferManager::create(ctx);
        textureMgr = TextureManager::create(ctx, bufferMgr, 10);
        swapchain = Swapchain::create(ctx, bufferMgr);
        renderer = Renderer::create(ctx, textureMgr, swapchain);

        atlasMgr = AtlasManager::create(textureMgr);

        // clang-format off
        debugCube2.texture = debugCube1.texture = textureMgr->createSimpleTexture("assets/debug.png", VK_FORMAT_R8G8B8A8_SRGB);
        debugCube2.mesh = debugCube1.mesh = bufferMgr->allocateSimpleMesh(
            {0,  1,  2,   2,  3,  0,
             4,  5,  6,   6,  7,  4,
             8,  9,  10,  10, 11, 8,
             12, 13, 14,  14, 15, 12,
             
             16, 17, 18,  18, 19, 16,
             20, 21, 22,  22, 23, 20},
            {{{-0.05f, -0.05f, 0.05f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
             {{ 0.05f, -0.05f, 0.05f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
             {{ 0.05f,  0.05f, 0.05f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
             {{-0.05f,  0.05f, 0.05f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
             
             {{ 0.05f, -0.05f, -0.05f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
             {{-0.05f, -0.05f, -0.05f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
             {{-0.05f,  0.05f, -0.05f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
             {{ 0.05f,  0.05f, -0.05f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
             
             {{-0.05f, 0.05f, -0.05f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
             {{ 0.05f, 0.05f, -0.05f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
             {{ 0.05f, 0.05f,  0.05f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
             {{-0.05f, 0.05f,  0.05f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
             
             {{ 0.05f, -0.05f, -0.05f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
             {{-0.05f, -0.05f, -0.05f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
             {{-0.05f, -0.05f,  0.05f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
             {{ 0.05f, -0.05f,  0.05f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
             
             {{0.05f, -0.05f, -0.05f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
             {{0.05f,  0.05f, -0.05f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
             {{0.05f,  0.05f,  0.05f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
             {{0.05f, -0.05f,  0.05f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
             
             {{-0.05f,  0.05f, -0.05f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
             {{-0.05f, -0.05f, -0.05f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
             {{-0.05f, -0.05f,  0.05f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
             {{-0.05f,  0.05f,  0.05f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}});
        // clang-format on
    } catch (...) {
        cleanup();
        throw;
    }
}

void PlayerController::cleanup() {
    // Wait for the device to finish rendering before cleaning up!
    ctx->waitDeviceIdle();

    if (atlasMgr) atlasMgr->cleanup();

    textureMgr->deallocateSimpleTexture(debugCube1.texture);
    bufferMgr->deallocateSimpleMesh(debugCube1.mesh);

    if (renderer) renderer->cleanup();

    if (swapchain) swapchain->cleanup();

    if (textureMgr) textureMgr->cleanup();

    if (bufferMgr) bufferMgr->cleanup();

    if (ctx) ctx->cleanup();

    Window::cleanup();
}

void PlayerController::onFrame(InputState &input) {
    handleInput(input);

    Camera camera{45.0f, 0.1f, 10.0f, pos, yaw, pitch};

    debugCube2.modelMatrix = glm::mat4(1.0f);
    debugCube1.modelMatrix = glm::rotate(
        glm::mat4(1.0f), getTime() * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    renderer->render(camera, {debugCube2, debugCube1}, windowResized);
    windowResized = false;
}

void PlayerController::onResize(int width, int height) { windowResized = true; }

void PlayerController::handleInput(InputState &input) {
    const float SENSIBILITY = 1e-3f;
    const float SPEED = 2.0f;

    glm::vec3 mov{0.0f, 0.0f, 0.0f};

    if (input.forward) {
        mov.z = 1.0f;
    }
    if (input.backward) {
        mov.z = -1.0f;
    }

    if (input.left) {
        mov.x = 1.0f;
    }
    if (input.right) {
        mov.x = -1.0f;
    }

    // Normalize movement vector
    if (glm::length(mov) > 0.1) mov = glm::normalize(mov);

    // Rotate according to current view angle
    mov = glm::rotate(mov, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    // Scale according to delta time and speed
    mov *= input.deltaTime * SPEED;

    // Add free flight
    if (input.jump) {
        mov.y = input.deltaTime;
    }
    if (input.crouch) {
        mov.y = -input.deltaTime;
    }

    pos += mov;
    yaw += input.viewDelta.x * SENSIBILITY;
    pitch += input.viewDelta.y * SENSIBILITY;

    // Clamp to prevent gimbal lock
    pitch = std::clamp(pitch, -(float)M_PI_2 + 1e-4f, (float)M_PI_2 - 1e-4f);
}