#pragma once

#include <memory>

#include "render/BufferManager.hpp"
#include "render/Context.hpp"
#include "render/Renderer.hpp"
#include "render/TextureManager.hpp"
#include "render/Window.hpp"
#include "world/AtlasManager.hpp"
#include "world/Chunk.hpp"
#include "world/Collision.hpp"
#include "world/World.hpp"

class PlayerController : public render::Window {
   public:
    PlayerController();

    void cleanup();

   protected:
    void onFrame(InputState &input) override;
    void onResize(int width, int height) override;

   private:
    void handleInput(InputState &input);

    void pushDebugCube(glm::vec3 pos, glm::quat rot);

    bool windowResized{false};

    world::World world{};

    float simulatedTime = 0.0f;
    float totalTime = 0.0f;
    world::SimulatedBoxCollider playerCollider{glm::vec3{0.0f, 14.0f, 0.0f},
                                               glm::vec3{0.8f, 1.8f, 0.8f}};

    float lastPlace{0.0f};
    float lastDestroy{0.0f};

    glm::vec3 pos{0.0f, 0.0f, 0.0f};
    float yaw{0.0f};
    float pitch{0.0f};
    glm::vec3 acc{0.0f, 0.0f, 0.0f};

    render::SimpleMesh debugCubeMesh;
    render::SimpleTexture debugTexture;

    // Per frame stuff
    std::list<render::SimpleModel> models;
};