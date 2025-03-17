#include "PlayerController.hpp"

#include <math.h>

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>

#include "VoxelRaytracer.hpp"

using namespace render;
using namespace world;

// clang-format off
std::vector<uint16_t> DEBUG_CUBE_INDICES = {
    0,  1,  2,  0,  2,  3,
    4,  5,  6,  4,  6,  7,

    8,  9,  10, 8,  10, 11,
    12, 13, 14, 12, 14, 15,

    16, 17, 18, 16, 18, 19,
    20, 21, 22, 20, 22, 23,
};

std::vector<Vertex> DEBUG_CUBE_VERTICES = {
    // Z- side
    {{+0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{-0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{+0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

    // Z+ side
    {{-0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{+0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{+0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

    // Bottom
    {{-0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{+0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{+0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

    // Top
    {{-0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{+0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{+0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

    // X- side
    {{-0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{-0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

    // X+ side
    {{+0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{+0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{+0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{+0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}
};
// clang-format on

PlayerController::PlayerController() : Window("UnnamedMinecraftClone") {
    try {
        Context::create(getWindow());
        BufferManager::create();
        TextureManager::create(10);
        Swapchain::create();
        Renderer::create();

        AtlasManager::create();

        debugTexture = TextureManager::get().createSimpleTexture(
            "assets/debug.png", VK_FORMAT_R8G8B8A8_SRGB);
        debugCubeMesh = BufferManager::get().allocateSimpleMesh(
            DEBUG_CUBE_INDICES, DEBUG_CUBE_VERTICES);

    } catch (...) {
        cleanup();
        throw;
    }
}

void PlayerController::cleanup() {
    world.cleanup();

    AtlasManager::destroy();

    TextureManager::get().deallocateSimpleTextureDefer(debugTexture);
    BufferManager::get().deallocateSimpleMeshDefer(debugCubeMesh);

    // Wait for the device to finish rendering before cleaning up!
    Context::get().waitDeviceIdle();

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

    if (!input.destroy) lastDestroy = 0.0f;
    if (!input.place) lastPlace = 0.0f;

    Camera camera{45.0f, 0.1f, 1000.0f, pos, glm::vec3{pitch, yaw, 0.0f}};
    utils::VoxelRaytracer raytracer(camera.pos, camera.computeViewDir());

    for (int i = 0; i < 20; i++) {
        auto hit = raytracer.getNextHit();
        if (hit.pos.y < 0 || hit.pos.y >= 16) break;

        auto block = world.getBlock(hit.pos);
        if (block != Block::AIR) {
            if (input.destroy && (getTime() > lastDestroy + 0.25f)) {
                lastDestroy = getTime();
                world.updateBlock(hit.pos, Block::AIR);
            }

            if (input.place && (getTime() > lastPlace + 0.25f)) {
                lastPlace = getTime();
                auto pos = hit.pos + hit.dir;
                world.updateBlock(pos, Block::DIAMOND);
            }

            pushDebugCube(camera.pos + camera.computeViewDir() * hit.dist,
                          glm::vec3(0.0f, 0.0f, 0.0f));
            break;
        }
    }

    world.getChunkInArea(camera.pos, 3, [this](glm::ivec3 pos, Chunk& chunk) {
        models.push_back(chunk.getModel(pos));
    });

    utils::VoxelRaytracer tracer(camera.pos, camera.computeViewDir());
    pushDebugCube(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    pushDebugCube(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    pushDebugCube(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    pushDebugCube(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    pushDebugCube(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    pushDebugCube(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

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