#include "MainWindow.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <memory>

#include "Random.hpp"
#include "glm/fwd.hpp"
#include "logic/DayNightCycle.hpp"
#include "render/BufferManager.hpp"
#include "render/GeometryRenderer.hpp"
#include "render/Renderer.hpp"
#include "render/Skybox.hpp"
#include "world/AtlasManager.hpp"
#include "world/Mucchina.hpp"
#include "world/Capretta.hpp"
#include "logic/HudManager.hpp"

using namespace render;
using namespace world;
using namespace logic;

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
    {{+0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
    {{-0.05f, -0.05f, -0.05f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
    {{-0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
    {{+0.05f, +0.05f, -0.05f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},

    // Z+ side
    {{-0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, +1.0f}, {0.0f, 1.0f}},
    {{+0.05f, -0.05f, +0.05f}, {0.0f, 0.0f, +1.0f}, {1.0f, 1.0f}},
    {{+0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, +1.0f}, {1.0f, 0.0f}},
    {{-0.05f, +0.05f, +0.05f}, {0.0f, 0.0f, +1.0f}, {0.0f, 0.0f}},

    // Bottom
    {{-0.05f, -0.05f, -0.05f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
    {{+0.05f, -0.05f, -0.05f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
    {{+0.05f, -0.05f, +0.05f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.05f, -0.05f, +0.05f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},

    // Top
    {{-0.05f, +0.05f, +0.05f}, {0.0f, +1.0f, 0.0f}, {0.0f, 1.0f}},
    {{+0.05f, +0.05f, +0.05f}, {0.0f, +1.0f, 0.0f}, {1.0f, 1.0f}},
    {{+0.05f, +0.05f, -0.05f}, {0.0f, +1.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.05f, +0.05f, -0.05f}, {0.0f, +1.0f, 0.0f}, {0.0f, 0.0f}},

    // X- side
    {{-0.05f, -0.05f, -0.05f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{-0.05f, -0.05f, +0.05f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.05f, +0.05f, +0.05f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.05f, +0.05f, -0.05f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

    // X+ side
    {{+0.05f, -0.05f, +0.05f}, {+1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{+0.05f, -0.05f, -0.05f}, {+1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{+0.05f, +0.05f, -0.05f}, {+1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{+0.05f, +0.05f, +0.05f}, {+1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}
};
// clang-format on

MainWindow::MainWindow() : Window{"UnnamedMinecraftClone", 10, 10} {
    atlas = AtlasManager::create();
    world = World::create(atlas);
    hudManager = HudManager::create(atlas);

    renderer = Renderer::create();
    mucchinaBlueprint = MucchinaBlueprint::create();
    caprettaBlueprint = CaprettaBlueprint::create();

    skybox = Skybox::make("assets/skybox_day.png", "assets/skybox_night.png");

    debugTexture = BufferManager::get().allocateTexture(
        "assets/debug.png", VK_FORMAT_R8G8B8A8_SRGB);
    debugCubeMesh = BufferManager::get().allocateMesh<GeometryMesh>(
        DEBUG_CUBE_INDICES, DEBUG_CUBE_VERTICES);

    playerController.unstuck(*world);
}

void MainWindow::onFrame(InputState& input) {
    models.clear();
    uiModels.clear();

    // Spawn mucchine as needed
    if (mucchine.size() < 50) {
        std::uniform_real_distribution<float> dist{-32.0f, 32.0f};

        glm::vec3 pos = playerController.getPos();
        pos.x += dist(RNG);
        pos.y = 0.0f;
        pos.z += dist(RNG);

        Mucchina mucchina = mucchinaBlueprint->fabricate();
        mucchina.teleport(pos);
        mucchina.unstuck(*world);

        mucchine.push_back(mucchina);
    }

    if (caprette.size() < 50) {
        std::uniform_real_distribution<float> dist{-32.0f, 32.0f};

        glm::vec3 pos = playerController.getPos();
        pos.x += dist(RNG);
        pos.y = 0.0f;
        pos.z += dist(RNG);

        Capretta capretta = caprettaBlueprint->fabricate();
        capretta.teleport(pos);
        capretta.unstuck(*world);

        caprette.push_back(capretta);
    }

    while ((input.time - simulatedTime) > 0.005f) {
        // Run physics at 100Hz
        playerController.update(*world, input);
        for (auto& mucchina : mucchine) {
            mucchina.update(*world);
        }
        for (auto& capretta : caprette) {
            capretta.update(*world);
        }
        simulatedTime += 0.005f;
    }

    // Destroy mucchine that are too far to be seen
    for (auto it = mucchine.begin(); it != mucchine.end(); it++) {
        glm::vec3 pos1 = playerController.getPos();
        glm::vec3 pos2 = it->getPos();
        if (std::abs(pos1.x - pos2.x) >= 48.0f ||
            std::abs(pos1.z - pos2.z) >= 48.0f)
            it = mucchine.erase(it);
    }

    for (auto it = caprette.begin(); it != caprette.end(); it++) {
        glm::vec3 pos1 = playerController.getPos();
        glm::vec3 pos2 = it->getPos();
        if (std::abs(pos1.x - pos2.x) >= 48.0f ||
            std::abs(pos1.z - pos2.z) >= 48.0f)
            it = caprette.erase(it);
    }

    auto dayNightState = logic::getDayNightState(input.time);

    world->getChunkInArea(playerController.getPos(), 3,
                          [this](glm::ivec3 pos, Chunk& chunk) {
                              models.push_back(chunk.getModel(pos));
                          });

    for (auto& mucchina : mucchine) mucchina.addToModelList(models);
    for (auto& capretta : caprette) capretta.addToModelList(models);


    hudManager->setSelectedBlock(input.selected_block);

    GeometryRenderer::LightInfo lights{dayNightState.ambientColor,
                                       dayNightState.sunDir,
                                       dayNightState.sunColor};

    skybox.lightDir = dayNightState.sunDir;
    skybox.blend = dayNightState.skyboxFade;

    hudManager->addToModelList(uiModels);

    renderer->render(playerController.getCamera(), skybox, lights, models,
                     uiModels, windowResized);
    windowResized = false;
}

void MainWindow::onResize(int width, int height) { windowResized = true; }

void MainWindow::pushDebugCube(glm::vec3 pos, glm::quat rot) {
    models.push_back(GeometryModel{debugCubeMesh, debugTexture, pos, rot});
}