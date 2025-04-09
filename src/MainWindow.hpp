#pragma once

#include <list>

#include "logic/AIController.hpp"
#include "logic/PlayerController.hpp"
#include "render/Window.hpp"
#include "world/World.hpp"

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
    logic::PlayerController playerController{glm::vec3{0.0f, 14.0f, 0.0f}};
    logic::AIController debugCubeController{glm::vec3{0.0f, 14.0f, 0.0f},
                                            glm::vec3{0.4f, 0.4f, 0.4f}};

    render::GeometryMesh debugCubeMesh;
    render::UiMesh uiMesh;
    render::Texture debugTexture;

    // Per frame stuff
    std::list<render::GeometryModel> models;
    std::list<render::UiModel> uiModels;
};