#include "MainWindow.hpp"

#include <math.h>

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>

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

MainWindow::MainWindow() : Window("UnnamedMinecraftClone") {
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

void MainWindow::cleanup() {
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

void MainWindow::onFrame(InputState& input) {
    models.clear();

    totalTime += input.deltaTime;
    while ((totalTime - simulatedTime) > 0.005f) {
        // Run physics at 100Hz
        playerController.update(world, input);
        debugCubeController.update(world);
        simulatedTime += 0.005f;
    }
    
    pushDebugCube(playerController.getLookingAt(), glm::vec3{0.0f, 0.0f, 0.0f});
    pushDebugCube(debugCubeController.getPos() + glm::vec3{0.0f, 0.05f, 0.0f}, glm::vec3{0.0f, 0.0f, 0.0f});

    world.getChunkInArea(playerController.getPos(), 3, [this](glm::ivec3 pos, Chunk& chunk) {
        models.push_back(chunk.getModel(pos));
    });

    Renderer::get().render(playerController.getCamera(), models, windowResized);
    windowResized = false;
}

void MainWindow::onResize(int width, int height) { windowResized = true; }

void MainWindow::pushDebugCube(glm::vec3 pos, glm::quat rot) {
    models.push_back(SimpleModel{debugCubeMesh, debugTexture, pos, rot});
}