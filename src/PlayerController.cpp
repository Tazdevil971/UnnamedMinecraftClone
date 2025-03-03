#include "PlayerController.hpp"

#include <math.h>

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>

#include "VoxelRaytracer.hpp"

using namespace render;
using namespace world;

PlayerController::PlayerController() : Window("UnnamedMinecraftClone") {
    try {
        Context::create(getWindow());
        BufferManager::create();
        TextureManager::create(10);
        Swapchain::create();
        Renderer::create();

        AtlasManager::create();

        // clang-format off
        debugTexture = TextureManager::get().createSimpleTexture("assets/debug.png", VK_FORMAT_R8G8B8A8_SRGB);
        debugCubeMesh = BufferManager::get().allocateSimpleMesh(
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
    Context::get().waitDeviceIdle();

    AtlasManager::destroy();

    TextureManager::get().deallocateSimpleTexture(debugTexture);
    BufferManager::get().deallocateSimpleMesh(debugCubeMesh);

    Renderer::destroy();
    Swapchain::destroy();
    TextureManager::destroy();
    BufferManager::destroy();
    Context::destroy();

    Window::cleanup();
}

void PlayerController::onFrame(InputState& input) {
    handleInput(input);
    models.clear();

    Camera camera{45.0f, 0.1f, 10.0f, pos, glm::vec3{pitch, yaw, 0.0f}};

    utils::VoxelRaytracer tracer(camera.pos, camera.computeViewDir());
    pushDebugCube(glm::vec3(1.0f, 0.0f, 0.0f),
                  glm::vec3(getTime() * glm::radians(45.0f), 0.0f, 0.0f));
    pushDebugCube(glm::vec3(-1.0f, 0.0f, 0.0f),
                  glm::vec3(getTime() * glm::radians(45.0f), 0.0f, 0.0f));
    pushDebugCube(glm::vec3(0.0f, 1.0f, 0.0f),
                  glm::vec3(getTime() * glm::radians(45.0f), 0.0f, 0.0f));
    pushDebugCube(glm::vec3(0.0f, -1.0f, 0.0f),
                  glm::vec3(getTime() * glm::radians(45.0f), 0.0f, 0.0f));
    pushDebugCube(glm::vec3(0.0f, 0.0f, 1.0f),
                  glm::vec3(getTime() * glm::radians(45.0f), 0.0f, 0.0f));
    pushDebugCube(glm::vec3(0.0f, 0.0f, -1.0f),
                  glm::vec3(getTime() * glm::radians(45.0f), 0.0f, 0.0f));

    Renderer::get().render(camera, models, windowResized);
    windowResized = false;
}

void PlayerController::onResize(int width, int height) { windowResized = true; }

void PlayerController::handleInput(InputState& input) {
    const float SENSIBILITY = 1e-3f;
    const float SPEED = 2.0f;

    glm::vec3 mov{0.0f, 0.0f, 0.0f};

    if (input.forward) {
        mov.z = -1.0f;
    }
    if (input.backward) {
        mov.z = 1.0f;
    }

    if (input.left) {
        mov.x = -1.0f;
    }
    if (input.right) {
        mov.x = 1.0f;
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
    pitch = std::clamp(pitch, -(float)M_PI_2 + 1e-6f, (float)M_PI_2 - 1e-6f);
}

void PlayerController::pushDebugCube(glm::vec3 pos, glm::quat rot) {
    models.push_back(SimpleModel{debugCubeMesh, debugTexture, pos, rot});
}