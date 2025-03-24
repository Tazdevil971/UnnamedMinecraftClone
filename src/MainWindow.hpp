#pragma once

#include <memory>

#include "render/BufferManager.hpp"
#include "render/Context.hpp"
#include "render/Renderer.hpp"
#include "render/TextureManager.hpp"
#include "render/Window.hpp"
#include "world/AtlasManager.hpp"
#include "world/Chunk.hpp"
#include "world/World.hpp"
#include "logic/AIController.hpp"
#include "logic/PlayerController.hpp"

class MainWindow : public render::Window {
   public:
    MainWindow();

    void cleanup();

   protected:
    void onFrame(InputState &input) override;
    void onResize(int width, int height) override;

   private:
    void pushDebugCube(glm::vec3 pos, glm::quat rot);

    bool windowResized{false};

    world::World world{};

    float simulatedTime = 0.0f;
    float totalTime = 0.0f;
    logic::PlayerController playerController{glm::vec3{0.0f, 14.0f, 0.0f}};
    logic::AIController debugCubeController{glm::vec3{0.0f, 14.0f, 0.0f},
                                            glm::vec3{0.4f, 0.4f, 0.4f}};

    render::SimpleMesh debugCubeMesh;
    render::SimpleTexture debugTexture;

    // Per frame stuff
    std::list<render::SimpleModel> models;
};