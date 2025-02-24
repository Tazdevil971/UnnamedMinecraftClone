#pragma once

#include <memory>

#include "render/BufferManager.hpp"
#include "render/Context.hpp"
#include "render/Renderer.hpp"
#include "render/TextureManager.hpp"
#include "render/Window.hpp"

#include "world/AtlasManager.hpp"

class PlayerController : public render::Window {
   public:
    PlayerController();

    void cleanup();

   protected:
    void onFrame(InputState &input) override;
    void onResize(int width, int height) override;

   private:
    void handleInput(InputState &input);

    bool windowResized{false};

    glm::vec3 pos{0.0f, 0.0f, 0.0f};
    float yaw{0.0f};
    float pitch{0.0f};

    std::shared_ptr<world::AtlasManager> atlasMgr;

    std::shared_ptr<render::Context> ctx;
    std::shared_ptr<render::Swapchain> swapchain;
    std::shared_ptr<render::BufferManager> bufferMgr;
    std::shared_ptr<render::TextureManager> textureMgr;
    std::shared_ptr<render::Renderer> renderer;

    render::SimpleModel model;
};