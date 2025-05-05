#pragma once

#include <list>

#include "logic/PlayerController.hpp"
#include "render/Renderer.hpp"
#include "render/Skybox.hpp"
#include "render/Window.hpp"
#include "world/Mucchina.hpp"
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

    std::unique_ptr<world::Mucchina> mucchina;

    std::unique_ptr<render::Renderer> renderer;
    render::Skybox skybox;

    render::GeometryMesh debugCubeMesh;
    render::UiMesh uiMesh;
    render::UiMesh pointerMesh;
    std::list<render::UiMesh> uiCubeMeshes;
    render::Texture debugTexture;
    render::Texture pointerTexture;
    render::Texture uiCubeTexture;

    // Per frame stuff
    std::list<render::GeometryModel> models;
    std::list<render::UiModel> uiModels;
};