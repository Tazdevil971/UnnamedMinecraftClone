#include "MainWindow.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "logic/DayNightCycle.hpp"
#include "render/BufferManager.hpp"
#include "render/Context.hpp"
#include "render/GeometryRenderer.hpp"
#include "render/Renderer.hpp"
#include "render/Skybox.hpp"
#include "render/Swapchain.hpp"
#include "world/AtlasManager.hpp"

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

std::vector<GeometryVertex> DEBUG_CUBE_VERTICES = {
    // Z- side
    {{+0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
    {{-0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
    {{-0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
    {{+0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},

    // Z+ side
    {{-0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, +1.0f}, {0.0f, 1.0f}},
    {{+0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, +1.0f}, {1.0f, 1.0f}},
    {{+0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, +1.0f}, {1.0f, 0.0f}},
    {{-0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, +1.0f}, {0.0f, 0.0f}},

    // Bottom
    {{-0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
    {{+0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
    {{+0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},

    // Top
    {{-0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, +1.0f, 0.0f}, {0.0f, 1.0f}},
    {{+0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, +1.0f, 0.0f}, {1.0f, 1.0f}},
    {{+0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, +1.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {0.0f, +1.0f, 0.0f}, {0.0f, 0.0f}},

    // X- side
    {{-0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{-0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

    // X+ side
    {{+0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {+1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{+0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {+1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{+0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, 0.0f}, {+1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{+0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, 0.0f}, {+1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}
};
// clang-format on

MainWindow::MainWindow() : Window("UnnamedMinecraftClone") {
    try {
        Context::create(getWindow());
        BufferManager::create(10, 10);
        AtlasManager::create();
        Swapchain::create();

        renderer = std::make_unique<Renderer>();

        skybox =
            Skybox::make("assets/skybox_day.png", "assets/skybox_night.png");

        debugTexture = BufferManager::get().allocateTexture(
            "assets/debug.png", VK_FORMAT_R8G8B8A8_SRGB);
        debugCubeMesh = BufferManager::get().allocateMesh<GeometryMesh>(
            DEBUG_CUBE_INDICES, DEBUG_CUBE_VERTICES);
        uiMesh = BufferManager::get().allocateMesh<UiMesh>({0, 1, 2, 3, 2, 1},
                                                           {
                                                               {{-600, 0}, {0, 0}},
                                                               {{-600, 200}, {0, 1}},
                                                               {{600, 0}, {1, 0}}, 
                                                               {{600,200}, {1,1}},
                                                           });
        pointerTexture = BufferManager::get().allocateTexture("assets/pointer_texture.png", VK_FORMAT_R8G8B8A8_SRGB);

        pointerMesh = BufferManager::get().allocateMesh<UiMesh>({0, 1, 2, 3, 2, 1},
                                                           {
                                                               {{-75, -75}, {0, 0}},
                                                               {{-75, 75}, {0, 1}},
                                                               {{75, -75}, {1, 0}}, 
                                                               {{75,75}, {1,1}},
                                                           });

        playerController.unstuck(world);
    } catch (...) {
        cleanup();
        throw;
    }
}

void MainWindow::cleanup() {
    world.cleanup();
    skybox.cleanup();

    AtlasManager::destroy();

    BufferManager::get().deallocateTextureDefer(debugTexture);
    BufferManager::get().deallocateTextureDefer(pointerTexture);
    BufferManager::get().deallocateMeshDefer(debugCubeMesh);
    BufferManager::get().deallocateMeshDefer(pointerMesh);
    BufferManager::get().deallocateMeshDefer(uiMesh);

    // Wait for the device to finish rendering before cleaning up!
    Context::get().waitDeviceIdle();

    if (renderer) {
        renderer->cleanup();
        renderer.reset();
    }

    Swapchain::destroy();
    BufferManager::destroy();
    Context::destroy();

    Window::cleanup();
}

void MainWindow::onFrame(InputState& input) {
    models.clear();
    uiModels.clear();

    while ((input.time - simulatedTime) > 0.005f) {
        // Run physics at 100Hz
        playerController.update(world, input);
        debugCubeController.update(world);
        simulatedTime += 0.005f;
    }

    auto dayNightState = logic::getDayNightState(input.time);

    pushDebugCube(playerController.getLookingAt(), glm::vec3{0.0f, 0.0f, 0.0f});
    pushDebugCube(debugCubeController.getPos() + glm::vec3{0.0f, 0.05f, 0.0f},
                  glm::vec3{0.0f, 0.0f, 0.0f});

    world.getChunkInArea(playerController.getPos(), 3,
                         [this](glm::ivec3 pos, Chunk& chunk) {
                             models.push_back(chunk.getModel(pos));
                         });

    glm::vec2 pos = {0, 0};

    uiModels.push_back(UiModel{pointerMesh, pointerTexture, pos});

    GeometryRenderer::LightInfo lights{dayNightState.ambientColor,
                                       dayNightState.sunDir,
                                       dayNightState.sunColor};

    skybox.rot = dayNightState.skyboxRot;
    skybox.blend = dayNightState.skyboxFade;

    renderer->render(playerController.getCamera(), skybox, lights, models,
                     uiModels, windowResized);
    windowResized = false;
}

void MainWindow::onResize(int width, int height) { windowResized = true; }

void MainWindow::pushDebugCube(glm::vec3 pos, glm::quat rot) {
    models.push_back(GeometryModel{debugCubeMesh, debugTexture, pos, rot});
}